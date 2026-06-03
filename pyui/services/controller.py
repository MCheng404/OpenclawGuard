from __future__ import annotations

import csv
import io
import re
import subprocess
from datetime import datetime
from pathlib import Path

from PyQt5.QtCore import QObject, pyqtSignal

from models import AppState


class AppController(QObject):
    state_changed = pyqtSignal(object)
    toast = pyqtSignal(str)

    def __init__(self) -> None:
        super().__init__()
        self.state = AppState()
        self.project_root = Path(r"D:\Open\OpenclawGuard")
        self.settings_file = self.project_root / "build-release" / "OpenclawGuard.ini"

    def refresh(self) -> None:
        self.state_changed.emit(self.state)

    def _read_ini_map(self) -> dict[str, str]:
        data: dict[str, str] = {}
        if not self.settings_file.exists():
            return data
        for line in self.settings_file.read_text(encoding="utf-8", errors="replace").splitlines():
            s = line.strip()
            if not s or s.startswith(";") or s.startswith("#") or s.startswith("["):
                continue
            if "=" in s:
                k, v = s.split("=", 1)
                data[k.strip()] = v.strip()
        return data

    def _write_ini_map(self, updates: dict[str, str], guard_prefix_only: bool = False) -> None:
        self.settings_file.parent.mkdir(parents=True, exist_ok=True)
        lines: list[str] = []
        if self.settings_file.exists():
            lines = self.settings_file.read_text(encoding="utf-8", errors="replace").splitlines()

        existing: dict[str, str] = {}
        for line in lines:
            s = line.strip()
            if "=" in s and not s.startswith("[") and not s.startswith(";") and not s.startswith("#"):
                k, v = s.split("=", 1)
                existing[k.strip()] = v.strip()

        if guard_prefix_only:
            existing = {k: v for k, v in existing.items() if not k.startswith("guardList\\")}
        existing.update(updates)

        output = ["[General]"]
        for k in sorted(existing.keys()):
            output.append(f"{k}={existing[k]}")

        self.settings_file.write_text("\n".join(output) + "\n", encoding="utf-8")

    def configure_gateway(self, cli_path: str, port: int) -> None:
        self.state.gateway_cli_path = (cli_path or "").strip()
        self.state.gateway_port = port
        self._write_ini_map({
            "gateway/path": self.state.gateway_cli_path,
            "gateway/port": str(port),
        })
        self.refresh()

    def load_settings(self) -> None:
        data = self._read_ini_map()
        self.state.gateway_cli_path = data.get("gateway/path", self.state.gateway_cli_path)
        try:
            self.state.gateway_port = int(data.get("gateway/port", str(self.state.gateway_port)))
        except ValueError:
            pass
        self.state.github_token = data.get("github/token", "")
        self.refresh()

    def set_github_token(self, token: str) -> None:
        self.state.github_token = token.strip()
        self._write_ini_map({"github/token": self.state.github_token})
        self.refresh()

    def _cli_base(self) -> list[str]:
        return [self.state.gateway_cli_path] if self.state.gateway_cli_path else ["openclaw"]

    def _run_cli(self, args: list[str], timeout: int = 20) -> tuple[bool, str]:
        cmd = self._cli_base() + args
        return self._run_cmd(cmd, timeout)

    def _run_cmd(self, cmd: list[str], timeout: int = 20) -> tuple[bool, str]:
        try:
            proc = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=timeout,
                encoding="utf-8",
                errors="replace",
            )
            out = (proc.stdout or "").strip()
            err = (proc.stderr or "").strip()
            msg = out if out else err
            return proc.returncode == 0, msg
        except FileNotFoundError:
            return False, f"命令不存在: {cmd[0]}"
        except subprocess.TimeoutExpired:
            return False, f"命令超时: {' '.join(cmd)}"
        except Exception as e:
            return False, f"执行失败: {e}"

    def refresh_gateway_status(self) -> None:
        ok, msg = self._run_cli(["gateway", "status"])
        raw = msg or ""
        text = raw.lower()

        m = re.search(r"port\s*=\s*(\d+)", raw, re.IGNORECASE)
        if not m:
            m = re.search(r"listening:\s*[^:]+:(\d+)", raw, re.IGNORECASE)
        if m:
            self.state.gateway_port = int(m.group(1))

        if ok and ("runtime: running" in text or "connectivity probe: ok" in text or "运行中" in raw):
            self.state.gateway_running = True
            self.state.gateway_status_text = "运行中"
        elif "not running" in text or "stopped" in text or "未启动" in raw or "离线" in raw:
            self.state.gateway_running = False
            self.state.gateway_status_text = "未启动"
        elif ok:
            self.state.gateway_status_text = "状态未知"
        else:
            self.state.gateway_status_text = "检测失败"

        self.refresh()

    def start_gateway(self) -> None:
        ok, msg = self._run_cli(["gateway", "start"], timeout=40)
        self.refresh_gateway_status()
        self.toast.emit("网关启动命令已执行" if ok else f"启动失败：{msg or '未知错误'}")

    def stop_gateway(self) -> None:
        ok, msg = self._run_cli(["gateway", "stop"], timeout=40)
        self.refresh_gateway_status()
        self.toast.emit("网关停止命令已执行" if ok else f"停止失败：{msg or '未知错误'}")

    def restart_gateway(self) -> None:
        ok, msg = self._run_cli(["gateway", "restart"], timeout=60)
        self.refresh_gateway_status()
        self.toast.emit("网关重启命令已执行" if ok else f"重启失败：{msg or '未知错误'}")

    def _parse_update_text(self, text: str) -> list[dict[str, str]]:
        rows: list[dict[str, str]] = []
        if not text:
            return rows

        current = re.search(r"Current version:\s*([^\r\n]+)", text, re.IGNORECASE)
        channel = re.search(r"Channel:\s*([^\r\n]+)", text, re.IGNORECASE)
        tag = re.search(r"Tag/spec:\s*([^\r\n]+)", text, re.IGNORECASE)

        if "up to date" in text.lower() or "no updates" in text.lower() or "no changes were applied" in text.lower():
            line = "已是最新或无变更"
        elif "available" in text.lower() or "planned actions" in text.lower():
            line = "检测到更新动作"
        else:
            line = "已完成检查"

        rows.append({
            "current": current.group(1).strip() if current else "未知",
            "channel": channel.group(1).strip() if channel else "未知",
            "target": tag.group(1).strip() if tag else "未知",
            "result": line,
        })
        return rows

    def fetch_updates(self) -> None:
        ok, msg = self._run_cli(["update", "--dry-run"], timeout=60)
        self.state.last_update_check_text = datetime.now().strftime("%H:%M:%S")

        rows = self._parse_update_text(msg or "")
        self.state.update_rows = rows
        self.state.update_count = 1 if (rows and "更新动作" in rows[0].get("result", "")) else 0
        self.refresh()
        self.toast.emit("更新检查完成" if ok else f"更新检查失败：{msg or '未知错误'}")

    def perform_update(self, channel_ui: str) -> None:
        mapping = {"稳定版": "stable", "测试版": "beta", "全部": ""}
        channel = mapping.get(channel_ui, "")
        args = ["update"]
        if channel:
            args += ["--channel", channel]
        ok, msg = self._run_cli(args, timeout=120)
        self.toast.emit("更新命令已执行" if ok else f"更新失败：{msg or '未知错误'}")
        self.fetch_updates()

    def refresh_guard_list(self, base_dir: str | None = None) -> None:
        root = Path(base_dir) if base_dir else self.project_root
        self.settings_file = root / "build-release" / "OpenclawGuard.ini"
        data = self._read_ini_map()

        groups: dict[int, dict[str, str]] = {}
        for k, v in data.items():
            m = re.match(r"guardList\\(\d+)\\(name|exePath|enabled)", k)
            if not m:
                continue
            idx = int(m.group(1))
            field = m.group(2)
            groups.setdefault(idx, {})[field] = v

        items: list[dict[str, str]] = []
        for idx in sorted(groups.keys()):
            g = groups[idx]
            name = g.get("name", "")
            p = g.get("exePath", "")
            en_raw = g.get("enabled", "true").lower()
            en = en_raw in ("1", "true")
            running = self._is_process_running(Path(p).name) if p else False
            items.append({
                "name": name,
                "path": p,
                "running": "运行中" if running else "未运行",
                "enabled": "是" if en else "否",
            })

        self.state.guard_rows = items
        self.state.guard_count = len(items)
        self.refresh()

    def save_guard_rows(self) -> None:
        updates: dict[str, str] = {}
        for i, row in enumerate(self.state.guard_rows):
            idx = i + 1
            updates[f"guardList\\{idx}\\name"] = row.get("name", "")
            updates[f"guardList\\{idx}\\exePath"] = row.get("path", "")
            updates[f"guardList\\{idx}\\enabled"] = "true" if row.get("enabled", "否") == "是" else "false"
        self._write_ini_map(updates, guard_prefix_only=True)

    def add_guard_item(self, name: str, exe_path: str) -> None:
        if not name.strip() or not exe_path.strip():
            self.toast.emit("名称和路径不能为空")
            return
        self.state.guard_rows.append({"name": name.strip(), "path": exe_path.strip(), "running": "未知", "enabled": "是"})
        self.save_guard_rows()
        self.refresh_guard_list()
        self.toast.emit("守护项已添加")

    def remove_guard_item(self, row_index: int) -> None:
        if row_index < 0 or row_index >= len(self.state.guard_rows):
            self.toast.emit("请选择要删除的守护项")
            return
        self.state.guard_rows.pop(row_index)
        self.save_guard_rows()
        self.refresh_guard_list()
        self.toast.emit("守护项已删除")

    def toggle_guard_enabled(self, row_index: int) -> None:
        if row_index < 0 or row_index >= len(self.state.guard_rows):
            self.toast.emit("请选择要切换的守护项")
            return
        cur = self.state.guard_rows[row_index].get("enabled", "否")
        self.state.guard_rows[row_index]["enabled"] = "否" if cur == "是" else "是"
        self.save_guard_rows()
        self.refresh_guard_list()
        self.toast.emit("守护项状态已切换")

    def replace_guard_rows(self, rows: list[dict[str, str]]) -> None:
        normalized: list[dict[str, str]] = []
        for row in rows:
            name = (row.get("name", "") or "").strip()
            path = (row.get("path", "") or "").strip()
            if not name and not path:
                continue
            enabled = (row.get("enabled", "是") or "是").strip()
            enabled = "否" if enabled in ("否", "false", "0", "False") else "是"
            running = (row.get("running", "未运行") or "未运行").strip()
            normalized.append({
                "name": name,
                "path": path,
                "running": running,
                "enabled": enabled,
            })

        self.state.guard_rows = normalized
        self.save_guard_rows()
        self.refresh_guard_list()
        self.toast.emit("守护表格修改已保存")

    def _is_process_running(self, exe_name: str) -> bool:
        if not exe_name:
            return False
        ok, out = self._run_cmd(["tasklist", "/FO", "CSV", "/NH"])
        if not ok:
            return False
        reader = csv.reader(io.StringIO(out))
        target = exe_name.lower()
        for row in reader:
            if row and row[0].strip().lower() == target:
                return True
        return False

    def detect_environment(self) -> None:
        rows: list[dict[str, str]] = []
        checks = [
            ("Node.js", ["node", "-v"]),
            ("npm", ["npm", "-v"]),
            ("Python", ["python", "--version"]),
            ("Git", ["git", "--version"]),
            ("Openclaw", ["openclaw", "--version"]),
            ("Qt/qmake", ["qmake", "-v"]),
        ]
        for name, cmd in checks:
            ok, out = self._run_cmd(cmd, timeout=8)
            version = (out.splitlines()[0].strip() if out else "-")
            rows.append({"name": name, "version": version, "status": "已安装" if ok else "未安装/不可用"})

        self.state.env_rows = rows
        self.state.env_last_check_text = datetime.now().strftime("%H:%M:%S")
        self.refresh()
        self.toast.emit("环境检测完成")


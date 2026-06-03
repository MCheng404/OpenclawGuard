from __future__ import annotations

from dataclasses import dataclass, field


@dataclass
class AppState:
    gateway_running: bool = False
    gateway_status_text: str = "未启动"
    gateway_port: int = 3456
    gateway_cli_path: str = ""

    guard_count: int = 0
    guard_rows: list[dict[str, str]] = field(default_factory=list)

    update_count: int = 0
    last_update_check_text: str = "未检查"
    update_rows: list[dict[str, str]] = field(default_factory=list)

    env_rows: list[dict[str, str]] = field(default_factory=list)
    env_last_check_text: str = "未检测"

    github_token: str = ""

    theme: str = "dark"


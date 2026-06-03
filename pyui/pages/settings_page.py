from __future__ import annotations

from PyQt5.QtWidgets import QHBoxLayout, QVBoxLayout, QWidget
from qfluentwidgets import BodyLabel, LineEdit, PrimaryPushButton, PushButton, SpinBox, StrongBodyLabel

from services.controller import AppController


class SettingsPage(QWidget):
    def __init__(self, controller: AppController, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.controller = controller

        root = QVBoxLayout(self)
        root.setContentsMargins(24, 24, 24, 24)
        root.setSpacing(12)

        root.addWidget(StrongBodyLabel("设置"))

        root.addWidget(BodyLabel("Openclaw CLI 路径"))
        self.cli_path = LineEdit(self)
        self.cli_path.setPlaceholderText("留空使用 PATH 中 openclaw")
        root.addWidget(self.cli_path)

        root.addWidget(BodyLabel("网关端口"))
        self.port_spin = SpinBox(self)
        self.port_spin.setRange(1, 65535)
        root.addWidget(self.port_spin)

        root.addWidget(BodyLabel("GitHub Token"))
        self.token_edit = LineEdit(self)
        self.token_edit.setPlaceholderText("用于 GitHub API 访问")
        self.token_edit.setClearButtonEnabled(True)
        root.addWidget(self.token_edit)

        btn_row = QHBoxLayout()
        save_btn = PrimaryPushButton("保存设置")
        reload_btn = PushButton("重新加载")
        save_btn.clicked.connect(self._save)
        reload_btn.clicked.connect(controller.load_settings)
        btn_row.addWidget(save_btn)
        btn_row.addWidget(reload_btn)
        btn_row.addStretch(1)
        root.addLayout(btn_row)

        root.addStretch(1)

        controller.state_changed.connect(self.on_state_changed)

    def _save(self) -> None:
        self.controller.configure_gateway(self.cli_path.text(), int(self.port_spin.value()))
        self.controller.set_github_token(self.token_edit.text())
        self.controller.toast.emit("设置已保存")

    def on_state_changed(self, state) -> None:
        self.cli_path.setText(state.gateway_cli_path)
        self.port_spin.setValue(state.gateway_port)
        self.token_edit.setText(state.github_token)


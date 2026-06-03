from __future__ import annotations

from PyQt5.QtWidgets import QHBoxLayout, QVBoxLayout, QWidget
from qfluentwidgets import BodyLabel, CaptionLabel, PrimaryPushButton, PushButton, StrongBodyLabel

from services.controller import AppController


class GatewayPage(QWidget):
    def __init__(self, controller: AppController, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.controller = controller

        root = QVBoxLayout(self)
        root.setContentsMargins(24, 24, 24, 24)
        root.setSpacing(12)

        root.addWidget(StrongBodyLabel("网关管理"))
        root.addWidget(CaptionLabel("管理 Openclaw 网关进程（配置入口统一在“设置”页）"))

        self.status_label = BodyLabel("状态：未启动")
        self.cfg_hint = BodyLabel("配置：CLI=PATH(openclaw), 端口=18789")

        btn_row = QHBoxLayout()
        self.start_btn = PrimaryPushButton("启动网关")
        self.stop_btn = PushButton("停止网关")
        self.restart_btn = PushButton("重启网关")
        self.refresh_btn = PushButton("刷新状态")

        self.start_btn.clicked.connect(controller.start_gateway)
        self.stop_btn.clicked.connect(controller.stop_gateway)
        self.restart_btn.clicked.connect(controller.restart_gateway)
        self.refresh_btn.clicked.connect(controller.refresh_gateway_status)

        btn_row.addWidget(self.start_btn)
        btn_row.addWidget(self.stop_btn)
        btn_row.addWidget(self.restart_btn)
        btn_row.addWidget(self.refresh_btn)

        root.addWidget(self.status_label)
        root.addWidget(self.cfg_hint)
        root.addLayout(btn_row)
        root.addStretch(1)

        controller.state_changed.connect(self.on_state_changed)

    def on_state_changed(self, state) -> None:
        cli = state.gateway_cli_path if state.gateway_cli_path else "PATH(openclaw)"
        self.status_label.setText(f"状态：{state.gateway_status_text}（端口 {state.gateway_port}）")
        self.cfg_hint.setText(f"配置：CLI={cli}, 端口={state.gateway_port}")


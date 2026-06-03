from __future__ import annotations

from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QHBoxLayout, QVBoxLayout, QWidget
from qfluentwidgets import BodyLabel, CaptionLabel, CardWidget, PrimaryPushButton, PushButton, StrongBodyLabel

from services.controller import AppController


class DashboardPage(QWidget):
    def __init__(self, controller: AppController, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.controller = controller

        root = QVBoxLayout(self)
        root.setContentsMargins(24, 24, 24, 24)
        root.setSpacing(16)

        title = StrongBodyLabel("仪表盘")
        desc = CaptionLabel("Openclaw Gateway 运行概览")
        root.addWidget(title)
        root.addWidget(desc)

        cards_row = QHBoxLayout()
        cards_row.setSpacing(12)

        self.gateway_card = self._stat_card("网关状态", "未启动")
        self.guard_card = self._stat_card("守护进程", "0")
        self.update_card = self._stat_card("可用更新", "0")

        cards_row.addWidget(self.gateway_card)
        cards_row.addWidget(self.guard_card)
        cards_row.addWidget(self.update_card)
        root.addLayout(cards_row)

        action_card = CardWidget(self)
        action_layout = QHBoxLayout(action_card)
        action_layout.setContentsMargins(16, 16, 16, 16)
        action_layout.setSpacing(8)

        start_btn = PrimaryPushButton("启动网关")
        stop_btn = PushButton("停止网关")
        check_btn = PushButton("检查更新")
        refresh_btn = PushButton("刷新状态")

        start_btn.clicked.connect(controller.start_gateway)
        stop_btn.clicked.connect(controller.stop_gateway)
        check_btn.clicked.connect(controller.fetch_updates)
        refresh_btn.clicked.connect(controller.refresh_gateway_status)

        action_layout.addWidget(start_btn)
        action_layout.addWidget(stop_btn)
        action_layout.addWidget(check_btn)
        action_layout.addWidget(refresh_btn)
        action_layout.addStretch(1)

        root.addWidget(action_card)
        root.addStretch(1)

        controller.state_changed.connect(self.on_state_changed)

    def _stat_card(self, label: str, value: str) -> CardWidget:
        card = CardWidget(self)
        card.setMinimumHeight(120)
        layout = QVBoxLayout(card)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(8)

        value_label = BodyLabel(value)
        value_label.setObjectName("valueLabel")
        text_label = CaptionLabel(label)

        layout.addWidget(value_label, 0, Qt.AlignLeft)
        layout.addWidget(text_label, 0, Qt.AlignLeft)
        layout.addStretch(1)

        card.value_label = value_label  # type: ignore[attr-defined]
        return card

    def on_state_changed(self, state) -> None:
        self.gateway_card.value_label.setText(state.gateway_status_text)
        self.guard_card.value_label.setText(str(state.guard_count))
        self.update_card.value_label.setText(str(state.update_count))


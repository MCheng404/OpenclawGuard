from __future__ import annotations

from PyQt5.QtWidgets import QHBoxLayout, QTableWidgetItem, QVBoxLayout, QWidget
from qfluentwidgets import BodyLabel, CaptionLabel, PrimaryPushButton, StrongBodyLabel, TableWidget

from services.controller import AppController


class EnvPage(QWidget):
    def __init__(self, controller: AppController, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.controller = controller

        root = QVBoxLayout(self)
        root.setContentsMargins(24, 24, 24, 24)
        root.setSpacing(12)

        root.addWidget(StrongBodyLabel("环境检测"))
        root.addWidget(CaptionLabel("检测 Python / Node / Qt / Openclaw 等依赖"))

        row = QHBoxLayout()
        detect_btn = PrimaryPushButton("开始检测")
        detect_btn.clicked.connect(controller.detect_environment)
        row.addWidget(detect_btn)
        row.addStretch(1)
        root.addLayout(row)

        self.last_check_label = BodyLabel("最近检测：未检测")
        root.addWidget(self.last_check_label)

        self.table = TableWidget(self)
        self.table.setColumnCount(3)
        self.table.setHorizontalHeaderLabels(["组件", "当前版本", "状态"])
        root.addWidget(self.table)

        controller.state_changed.connect(self.on_state_changed)

    def on_state_changed(self, state) -> None:
        self.last_check_label.setText(f"最近检测：{state.env_last_check_text}")
        rows = state.env_rows
        self.table.setRowCount(len(rows))
        for i, row in enumerate(rows):
            self.table.setItem(i, 0, QTableWidgetItem(row.get("name", "")))
            self.table.setItem(i, 1, QTableWidgetItem(row.get("version", "")))
            self.table.setItem(i, 2, QTableWidgetItem(row.get("status", "")))


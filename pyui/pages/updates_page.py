from __future__ import annotations

from PyQt5.QtWidgets import QHBoxLayout, QMessageBox, QTableWidgetItem, QVBoxLayout, QWidget
from qfluentwidgets import BodyLabel, CaptionLabel, ComboBox, PrimaryPushButton, ProgressBar, PushButton, StrongBodyLabel, TableWidget

from services.controller import AppController


class UpdatesPage(QWidget):
    def __init__(self, controller: AppController, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.controller = controller

        root = QVBoxLayout(self)
        root.setContentsMargins(24, 24, 24, 24)
        root.setSpacing(12)

        root.addWidget(StrongBodyLabel("更新管理"))
        root.addWidget(CaptionLabel("检查 Openclaw 更新与 GitHub Releases"))

        top_row = QHBoxLayout()
        self.channel_combo = ComboBox(self)
        self.channel_combo.addItems(["稳定版", "测试版", "全部"])
        self.check_btn = PrimaryPushButton("检查更新")
        self.run_btn = PushButton("执行更新")
        self.check_btn.clicked.connect(controller.fetch_updates)
        self.run_btn.clicked.connect(self._run_update)
        top_row.addWidget(self.channel_combo)
        top_row.addWidget(self.check_btn)
        top_row.addWidget(self.run_btn)
        top_row.addStretch(1)
        root.addLayout(top_row)

        self.last_check_label = BodyLabel("最近检查：未检查")
        root.addWidget(self.last_check_label)

        self.progress = ProgressBar(self)
        self.progress.setRange(0, 100)
        self.progress.setValue(100)
        root.addWidget(self.progress)

        self.table = TableWidget(self)
        self.table.setColumnCount(4)
        self.table.setHorizontalHeaderLabels(["当前版本", "通道", "目标规格", "结果"])
        root.addWidget(self.table)

        controller.state_changed.connect(self.on_state_changed)

    def _run_update(self) -> None:
        channel_ui = self.channel_combo.currentText()
        ok = QMessageBox.question(self, "确认更新", f"确认执行 Openclaw 更新？\n通道：{channel_ui}")
        if ok == QMessageBox.Yes:
            self.controller.perform_update(channel_ui)

    def on_state_changed(self, state) -> None:
        self.last_check_label.setText(f"最近检查：{state.last_update_check_text}")
        rows = state.update_rows
        self.table.setRowCount(len(rows))
        for i, row in enumerate(rows):
            self.table.setItem(i, 0, QTableWidgetItem(row.get("current", "")))
            self.table.setItem(i, 1, QTableWidgetItem(row.get("channel", "")))
            self.table.setItem(i, 2, QTableWidgetItem(row.get("target", "")))
            self.table.setItem(i, 3, QTableWidgetItem(row.get("result", "")))


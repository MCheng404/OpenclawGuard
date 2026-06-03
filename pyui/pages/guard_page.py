from __future__ import annotations

from PyQt5.QtWidgets import QFileDialog, QHBoxLayout, QTableWidgetItem, QVBoxLayout, QWidget
from qfluentwidgets import BodyLabel, CaptionLabel, PrimaryPushButton, PushButton, StrongBodyLabel, TableWidget, LineEdit

from services.controller import AppController


class GuardPage(QWidget):
    def __init__(self, controller: AppController, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.controller = controller

        root = QVBoxLayout(self)
        root.setContentsMargins(24, 24, 24, 24)
        root.setSpacing(12)

        root.addWidget(StrongBodyLabel("进程守护"))
        root.addWidget(CaptionLabel("监控关键进程，崩溃自动拉起（表格可直接编辑后保存）"))

        add_row = QHBoxLayout()
        self.name_edit = LineEdit(self)
        self.name_edit.setPlaceholderText("守护项名称")
        self.path_edit = LineEdit(self)
        self.path_edit.setPlaceholderText("可执行文件路径")
        browse_btn = PushButton("浏览")
        browse_btn.clicked.connect(self._browse_exe)
        add_btn = PrimaryPushButton("添加")
        add_btn.clicked.connect(self._add_item)
        add_row.addWidget(self.name_edit)
        add_row.addWidget(self.path_edit)
        add_row.addWidget(browse_btn)
        add_row.addWidget(add_btn)
        root.addLayout(add_row)

        btn_row = QHBoxLayout()
        refresh_btn = PrimaryPushButton("刷新守护列表")
        save_btn = PushButton("保存表格修改")
        toggle_btn = PushButton("启用/禁用")
        remove_btn = PushButton("删除选中")
        refresh_btn.clicked.connect(controller.refresh_guard_list)
        save_btn.clicked.connect(self._save_table)
        toggle_btn.clicked.connect(self._toggle_selected)
        remove_btn.clicked.connect(self._remove_selected)
        btn_row.addWidget(refresh_btn)
        btn_row.addWidget(save_btn)
        btn_row.addWidget(toggle_btn)
        btn_row.addWidget(remove_btn)
        btn_row.addStretch(1)
        root.addLayout(btn_row)

        self.count_label = BodyLabel("守护项：0")
        root.addWidget(self.count_label)

        self.table = TableWidget(self)
        self.table.setColumnCount(4)
        self.table.setHorizontalHeaderLabels(["名称", "路径", "状态", "启用(是/否)"])
        self.table.setEditTriggers(self.table.EditTrigger.DoubleClicked | self.table.EditTrigger.EditKeyPressed)
        root.addWidget(self.table)

        controller.state_changed.connect(self.on_state_changed)

    def _browse_exe(self) -> None:
        path, _ = QFileDialog.getOpenFileName(self, "选择可执行文件", "", "Executable (*.exe);;All Files (*)")
        if path:
            self.path_edit.setText(path)

    def _add_item(self) -> None:
        self.controller.add_guard_item(self.name_edit.text(), self.path_edit.text())

    def _selected_row(self) -> int:
        idxs = self.table.selectionModel().selectedRows() if self.table.selectionModel() else []
        return idxs[0].row() if idxs else -1

    def _toggle_selected(self) -> None:
        self.controller.toggle_guard_enabled(self._selected_row())

    def _remove_selected(self) -> None:
        self.controller.remove_guard_item(self._selected_row())

    def _save_table(self) -> None:
        rows: list[dict[str, str]] = []
        for r in range(self.table.rowCount()):
            name = self.table.item(r, 0).text().strip() if self.table.item(r, 0) else ""
            path = self.table.item(r, 1).text().strip() if self.table.item(r, 1) else ""
            running = self.table.item(r, 2).text().strip() if self.table.item(r, 2) else "未运行"
            enabled = self.table.item(r, 3).text().strip() if self.table.item(r, 3) else "是"
            enabled = "是" if enabled not in ("否", "false", "0", "False") else "否"
            if name or path:
                rows.append({"name": name, "path": path, "running": running, "enabled": enabled})
        self.controller.replace_guard_rows(rows)

    def on_state_changed(self, state) -> None:
        rows = state.guard_rows
        self.count_label.setText(f"守护项：{len(rows)}")
        self.table.setRowCount(len(rows))
        for i, row in enumerate(rows):
            self.table.setItem(i, 0, QTableWidgetItem(row.get("name", "")))
            self.table.setItem(i, 1, QTableWidgetItem(row.get("path", "")))
            self.table.setItem(i, 2, QTableWidgetItem(row.get("running", "")))
            self.table.setItem(i, 3, QTableWidgetItem(row.get("enabled", "")))


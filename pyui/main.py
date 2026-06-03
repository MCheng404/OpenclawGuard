from __future__ import annotations

from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QApplication
from qfluentwidgets import FluentIcon as FIF, NavigationItemPosition, setTheme, Theme
from qfluentwidgets import FluentWindow, InfoBar, InfoBarPosition

from pages.dashboard_page import DashboardPage
from pages.gateway_page import GatewayPage
from pages.guard_page import GuardPage
from pages.updates_page import UpdatesPage
from pages.env_page import EnvPage
from pages.settings_page import SettingsPage
from services.controller import AppController


class MainWindow(FluentWindow):
    def __init__(self) -> None:
        super().__init__()
        self.controller = AppController()
        self.controller.toast.connect(self.show_toast)

        self.dashboard = DashboardPage(self.controller, self)
        self.gateway = GatewayPage(self.controller, self)
        self.guard = GuardPage(self.controller, self)
        self.updates = UpdatesPage(self.controller, self)
        self.env = EnvPage(self.controller, self)
        self.settings = SettingsPage(self.controller, self)

        self.addSubInterface(self.dashboard, FIF.HOME, "仪表盘")
        self.addSubInterface(self.gateway, FIF.ROBOT, "网关管理")
        self.addSubInterface(self.guard, FIF.DEVELOPER_TOOLS, "进程守护")
        self.addSubInterface(self.updates, FIF.UPDATE, "更新管理")
        self.addSubInterface(self.env, FIF.IOT, "环境检测")
        self.addSubInterface(self.settings, FIF.SETTING, "设置", NavigationItemPosition.BOTTOM)

        self.resize(1100, 720)
        self.setWindowTitle("OpenclawGuard - Fluent UI")
        self.controller.load_settings()
        self.controller.refresh_gateway_status()
        self.controller.refresh_guard_list()
        self.controller.detect_environment()
        self.controller.fetch_updates()
        self.controller.refresh()

    def show_toast(self, text: str) -> None:
        InfoBar.success(
            title="OpenclawGuard",
            content=text,
            orient=Qt.Horizontal,
            isClosable=True,
            position=InfoBarPosition.TOP,
            duration=1800,
            parent=self,
        )


if __name__ == "__main__":
    app = QApplication([])
    setTheme(Theme.DARK)
    w = MainWindow()
    w.show()
    app.exec()


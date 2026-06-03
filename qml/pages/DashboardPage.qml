import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property bool isDark: backend.themeName === "dark"

    Rectangle {
        anchors { fill: parent; margins: 24 }
        color: "transparent"

        ColumnLayout {
            anchors.fill: parent
            spacing: 16

            // Header
            RowLayout {
                ColumnLayout {
                    Label { text: "仪表盘"; font.pixelSize: 22; font.bold: true
                        color: isDark ? "#e2e4f0" : "#1e293b" }
                    Label { text: "Openclaw Gateway 运行概览"; font.pixelSize: 12
                        color: isDark ? "#8b8fa3" : "#64748b" }
                }
                Item { Layout.fillWidth: true }
            }

            // Stat cards
            RowLayout {
                spacing: 16
                Layout.fillWidth: true

                Repeater {
                    model: [
                    { icon: "◎", label: "网关状态", value: backend.gateway.isGatewayRunning() ? "运行中" : "未启动" },
                    { icon: "◈", label: "守护进程", value: backend.guard.itemCount },
                    { icon: "⇓", label: "可用更新", value: backend.updater.stableReleases.length + backend.updater.betaReleases.length }
                    ]
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120
                        radius: 16
                        color: isDark ? "#222540" : "#ffffff"
                        border { width: 1; color: isDark ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06) }

                        ColumnLayout {
                            anchors { fill: parent; margins: 20 }
                            spacing: 8

                            RowLayout {
                                Label { text: modelData.icon; font.pixelSize: 18
                                    color: backend.accent }
                                Item { Layout.fillWidth: true }
                                Rectangle {
                                    width: 10; height: 10; radius: 5
                                    color: modelData.icon === "◎"
                                        ? (backend.gateway.isGatewayRunning() ? "#34d399" : "#555")
                                        : backend.accent
                                }
                            }
                            Label { text: modelData.value; font.pixelSize: 28; font.weight: Font.Light
                                color: isDark ? "#e2e4f0" : "#1e293b" }
                            Label { text: modelData.label; font.pixelSize: 12
                                color: isDark ? "#8b8fa3" : "#64748b" }
                        }
                    }
                }
            }

            // Quick actions
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                radius: 16
                color: isDark ? "#222540" : "#ffffff"
                border { width: 1; color: isDark ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06) }

                RowLayout {
                    anchors { fill: parent; margins: 20 }
                    spacing: 8

                    Label { text: "快捷操作"; font.pixelSize: 14; font.bold: true
                        color: isDark ? "#e2e4f0" : "#1e293b" }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "启动网关"
                        flat: true
                        onClicked: backend.gateway.startGateway()
                    }
                    Button {
                        text: "停止网关"
                        flat: true
                        onClicked: backend.gateway.stopGateway()
                    }
                    Button {
                        text: "检查更新"
                        flat: true
                        onClicked: backend.updater.fetchReleases()
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    property bool isDark: backend.themeName === "dark"

    Rectangle { anchors { fill: parent; margins: 24 }; color: "transparent"
    ColumnLayout { anchors.fill: parent; spacing: 16
        Label { text: "网关管理"; font.pixelSize: 22; font.bold: true; color: isDark ? "#e2e4f0" : "#1e293b" }
        Label { text: "启动、停止、重启 Openclaw Gateway"; font.pixelSize: 12; color: isDark ? "#8b8fa3" : "#64748b" }

        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 120; radius: 16
            color: isDark ? "#222540" : "#ffffff"
            border { width: 1; color: isDark ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06) }
            RowLayout { anchors { fill: parent; margins: 20 }; spacing: 12
                Label { text: "状态:"; color: isDark ? "#8b8fa3" : "#64748b" }
                Label { text: backend.gateway.isGatewayRunning() ? "运行中" : "已停止"
                    color: backend.gateway.isGatewayRunning() ? "#34d399" : "#ef4444" }
                Item { Layout.fillWidth: true }
                Button { text: backend.gateway.isGatewayRunning() ? "停止" : "启动"
                    onClicked: backend.gateway.isGatewayRunning() ? backend.gateway.stopGateway() : backend.gateway.startGateway() }
                Button { text: "重启"; onClicked: backend.gateway.restartGateway() }
            }
        }
        Item { Layout.fillHeight: true }
    }}
}

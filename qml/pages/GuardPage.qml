import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    property bool isDark: backend.themeName === "dark"
    Rectangle { anchors { fill: parent; margins: 24 }; color: "transparent"
    ColumnLayout { anchors.fill: parent; spacing: 16
        Label { text: "进程守护"; font.pixelSize: 22; font.bold: true; color: isDark ? "#e2e4f0" : "#1e293b" }
        Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; radius: 16; color: isDark ? "#222540" : "#ffffff"
            Label { anchors.centerIn: parent; text: "守护进程列表"; color: isDark ? "#8b8fa3" : "#64748b" }
        }
    }}
}

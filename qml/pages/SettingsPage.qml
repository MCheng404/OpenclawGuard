import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    property bool isDark: backend.themeName === "dark"
    Rectangle { anchors { fill: parent; margins: 24 }; color: "transparent"
    ColumnLayout { anchors.fill: parent; spacing: 16
        Label { text: "设置"; font.pixelSize: 22; font.bold: true; color: isDark ? "#e2e4f0" : "#1e293b" }
        Rectangle { Layout.fillWidth: true; radius: 16; color: isDark ? "#222540" : "#ffffff"
            border { width: 1; color: isDark ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.06) }
            ColumnLayout { anchors { fill: parent; margins: 20 }; spacing: 16
                Label { text: "外观"; font.pixelSize: 14; font.bold: true; color: isDark ? "#e2e4f0" : "#1e293b" }
                RowLayout {
                    Label { text: "主题:"; color: isDark ? "#8b8fa3" : "#64748b" }
                    ComboBox {
                        model: ["跟随系统", "浅色", "深色"]
                        currentIndex: 2
                        onCurrentIndexChanged: {
                            const t = ["system", "light", "dark"]
                            backend.setTheme(t[currentIndex])
                        }
                    }
                }
            }
        }
        Item { Layout.fillHeight: true }
    }}
}

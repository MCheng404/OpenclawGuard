import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true; width: 1020; height: 660; title: "OpenclawGuard"; color: "#141425"
    RowLayout { anchors.fill: parent; spacing: 0
        Rectangle { Layout.preferredWidth: 200; Layout.fillHeight: true; color: "#20233a"
            ColumnLayout { anchors.fill: parent
                Label { text: "  OpenclawGuard"; font { pixelSize: 15; bold: true }; color: "#4f8cff"; Layout.topMargin: 16 }
                ItemDelegate { Layout.fillWidth: true; height: 42
                    background: Rectangle { radius: 10; color: "#1a4f8cff" }
                    contentItem: Label { text: "仪表盘"; color: "#4f8cff"; font.pixelSize: 13; verticalAlignment: Text.AlignVCenter }
                    onClicked: pageView.currentIndex = 0 }
                ItemDelegate { Layout.fillWidth: true; height: 42
                    background: Rectangle { radius: 10; color: "transparent" }
                    contentItem: Label { text: "网关管理"; color: "#e2e4f0"; font.pixelSize: 13; verticalAlignment: Text.AlignVCenter }
                    onClicked: pageView.currentIndex = 1 }
                Item { Layout.fillHeight: true }
            }
        }
        Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: "#10ffffff" }
        StackLayout { id: pageView; Layout.fillWidth: true; Layout.fillHeight: true; currentIndex: 0
            Item { Rectangle { anchors.fill: parent; color: "#141425"; Label { anchors.centerIn: parent; text: "仪表盘"; color: "#e2e4f0"; font.pixelSize: 24 } }}
            Item { Rectangle { anchors.fill: parent; color: "#141425"; Label { anchors.centerIn: parent; text: "网关"; color: "#e2e4f0"; font.pixelSize: 24 } }}
        }
    }
}

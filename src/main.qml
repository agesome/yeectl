import Qt.labs.platform
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import yeectl

Window {
    id: window
    flags: Qt.Popup

    height: frame.height
    width: frame.width

    y: Screen.desktopAvailableHeight - height
    x: Screen.desktopAvailableWidth - width

    onActiveChanged: window.visible = active

    Frame {
        id: frame
        background: Rectangle {
            border.color: "black"
        }

        RowLayout {
            height: slider.height

            Switch {
                checked: DeviceManager.power
                onCheckedChanged: DeviceManager.power = checked
            }

            ToolSeparator {}

            Label {
                Layout.maximumHeight: parent.height
                Layout.maximumWidth: 0.3 * slider.width
                fontSizeMode: Text.Fit
                font.pointSize: 100
                horizontalAlignment: Text.AlignRight
                text: slider.value
            }

            Slider {
                id: slider
                value: DeviceManager.brightness
                from: 0
                to: 100
                stepSize: 5
                snapMode: Slider.SnapAlways
                wheelEnabled: true
                live: false
                onValueChanged: DeviceManager.brightness = value
            }
        }
    }

    SystemTrayIcon {
        id: tray
        visible: true
        icon.source: "qrc:/icons/lamp.svg"

        menu: Menu {
            MenuItem {
                text: qsTr("Quit")
                onTriggered: Qt.quit()
            }
        }

        onActivated: reason => {
                         if (reason === SystemTrayIcon.Trigger) {
                             window.show()
                         }
                     }
    }
}

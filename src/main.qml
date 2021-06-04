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
//    y: tray.geometry.y // - tray.geometry.height - height
//    x: tray.geometry.x //- tray.geometry.width - width

//    onXChanged: {
//        console.log('pos: ', x, y)
//    }

//    Component.onCompleted: {
//        console.log(x, y)
//    }

    onActiveChanged: window.visible = active

    Frame {
        id: frame
        background: Rectangle {
            border.color: "black"
        }

        RowLayout {
            height: slider.height

            Switch {
            }

            ToolSeparator {}

            Label {
                Layout.maximumHeight: parent.height
                Layout.maximumWidth: 0.3 * slider.width//textMetrics.width
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
                onValueChanged: DeviceManager.brightness = value
            }
        }
    }

    SystemTrayIcon {
        id: tray
        visible: true
        icon.source: "qrc:/icons/lamp.svg"

        menu: Menu {
            //            MenuItem {
            //                text: qsTr("Toggle")
            //                onTriggered: deviceList.currentDevice.toggle()
            //            }
            MenuItem {
                text: qsTr("Quit")
                onTriggered: Qt.quit()
            }
        }

        onActivated: (reason) => {
                         if (reason === SystemTrayIcon.Trigger) {
//                             window.y = tray.geometry.y - window.height - tray.geometry.height - 150
//                             window.x = tray.geometry.x - window.width - tray.geometry.width - 150
//                             console.log(tray.geometry)
//                             console.log(window.x, window.y)
                             window.show()
                         }
                     }
    }

}


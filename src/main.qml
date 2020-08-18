import Qt.labs.platform 1.1
import QtQuick.Window 2.2
import QtQuick.Controls 2.15 as C

import yeectl 1.0

Window {
    id: window
    flags: Qt.Popup

    x: tray.geometry.x
    y: tray.geometry.y - height
    width: slider.width
    height: slider.height

    onActiveChanged: window.visible = active

    DeviceList {
        id: deviceList
    }

    C.Slider {
        id: slider
        from: 1
        value: 25
        to: 100
    }

    SystemTrayIcon {
        id: tray
        visible: true
        icon.source: "qrc:/icons/lamp.svg"

        menu: Menu {
            MenuItem {
                text: qsTr("Toggle")
                onTriggered: deviceList.currentDevice.toggle()
            }
            MenuItem {
                text: qsTr("Quit")
                onTriggered: Qt.quit()
            }
        }

        onActivated: if (reason === SystemTrayIcon.Trigger) {
                         window.show()
                     }
    }

}


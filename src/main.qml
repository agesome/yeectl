import Qt.labs.platform
import QtQuick.Window
import QtQuick.Controls

import yeectl

Window {
    id: window
//    flags: Qt.Popup

//    x: tray.geometry.x
//    y: tray.geometry.y - height
    width: 100//slider.width
    height: 100//slider.height
    visible: true

//    onActiveChanged: window.visible = active

//    DeviceList {
//        id: deviceList
//    }

    Slider {
        id: slider
        from: 1
        value: 0
        stepSize: 1
        width: 100
        height: 100
        to: 100
        onValueChanged: DeviceManager.set_brightness(value)
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
                         console.log("activated", reason)
//                         if (reason === SystemTrayIcon.Trigger) {
                             window.show()
//                         }
                     }
    }

}


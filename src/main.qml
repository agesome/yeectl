import Qt.labs.platform 1.1

//import yeectl.Device 1.0
import yeectl 1.0

SystemTrayIcon {
    visible: true
    icon.source: "qrc:/lamp.ico"

    menu: Menu {
        DeviceList {
            id: deviceList
        }
        MenuItem {
            text: qsTr("Quit")
            onTriggered: Qt.quit()
        }
        MenuItem {
            text: qsTr("Toggle")
            onTriggered: deviceList.currentDevice.toggle()
        }
    }
    onActivated: console.log("hello")
}

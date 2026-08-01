import QtQuick
import QtQuick.Layouts

// online/offline indicator + pending-sync counter.
RowLayout {
    id: root
    spacing: 12

    Rectangle {
        width: 10
        height: 10
        radius: 5
        color: controller.online ? "green" : "red"
    }

    Text {
        text: controller.online ? "online" : "offline"
    }

    Item { Layout.fillWidth: true }

    Text {
        text: controller.pendingSyncCount + " do synchronizacji"
    }
}

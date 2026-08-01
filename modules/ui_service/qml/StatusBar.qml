import QtQuick
import QtQuick.Layouts

// online/offline indicator + pending-sync counter. Bind to
// controller.online / controller.pendingSyncCount — left as TODO.
RowLayout {
    id: root
    spacing: 12

    Rectangle {
        width: 10
        height: 10
        radius: 5
        color: "gray"  // TODO: color: controller.online ? "green" : "red"
    }

    Text {
        text: "offline"  // TODO: text: controller.online ? "online" : "offline"
    }

    Item { Layout.fillWidth: true }

    Text {
        text: "0 do synchronizacji"  // TODO: text: controller.pendingSyncCount + " do synchronizacji"
    }
}

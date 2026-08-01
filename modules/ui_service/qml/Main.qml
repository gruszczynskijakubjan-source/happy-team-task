import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 480
    height: 640
    visible: true
    title: "Automat vendingowy"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        StatusBar {
            Layout.fillWidth: true
        }

        ProductGrid {
            Layout.fillWidth: true
            Layout.fillHeight: true
            // Only selectable once a card has been tapped (CardRead); grayed
            // out in Idle, and again once a product is picked (Dispensing)
            // or the selection timeout returns the engine to Idle.
            enabled: controller.stateName === "CardRead"
        }

        Button {
            text: "Symuluj przyłożenie karty"
            Layout.fillWidth: true
            onClicked: controller.simulateCardTap()
        }

        ProgressBar {
            id: dispenseProgress
            Layout.fillWidth: true
            visible: controller.dispensing
            from: 0
            to: 100
            value: controller.dispenseProgress
        }
    }
}

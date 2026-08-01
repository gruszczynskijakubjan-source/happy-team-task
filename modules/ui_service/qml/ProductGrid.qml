import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Static 3-4 product grid. Selection calls controller.selectProduct(productId).
GridView {
    id: root
    cellWidth: width / 2
    cellHeight: 120

    model: ListModel {
        ListElement { productId: "coke_330"; name: "Cola 330ml" }
        ListElement { productId: "water_500"; name: "Woda 500ml" }
        ListElement { productId: "chips_snack"; name: "Chipsy" }
        ListElement { productId: "chocolate_bar"; name: "Baton" }
    }

    delegate: ItemDelegate {
        width: root.cellWidth
        height: root.cellHeight
        text: name
        onClicked: controller.selectProduct(productId)
    }
}

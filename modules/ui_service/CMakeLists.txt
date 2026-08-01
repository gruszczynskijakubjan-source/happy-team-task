find_package(Qt6 COMPONENTS Core Gui Qml Quick QuickControls2 REQUIRED)

set(CMAKE_AUTOMOC ON)

# ui_service is the Qt/QML facade only: VendingController plus the QML
# module. Object construction/wiring lives in the root main.cpp
# (composition root), which links against this library.
qt_add_library(ui_service STATIC
  VendingController.cpp
)

qt_add_qml_module(ui_service
  URI VendingApp
  VERSION 1.0
  QML_FILES
    qml/Main.qml
    qml/ProductGrid.qml
    qml/StatusBar.qml
)

target_include_directories(ui_service PUBLIC ${CMAKE_SOURCE_DIR}/modules ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ui_service PUBLIC
  vending::vending_engine_service
  vending::cloud_service
  vending::rfid_service
  vending::shared_helper
  Qt6::Core
  Qt6::Gui
  Qt6::Qml
  Qt6::Quick
  Qt6::QuickControls2
)

add_library(vending::ui_service ALIAS ui_service)

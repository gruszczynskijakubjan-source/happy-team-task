cmake_minimum_required(VERSION 3.16)
project(vending_machine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

option(VENDING_BUILD_TESTS "Build unit tests" ON)
option(VENDING_BUILD_APP "Build Qt/QML application (ui_service)" OFF)

find_package(Threads REQUIRED)

add_compile_options(
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wextra>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wpedantic>
)

if(VENDING_BUILD_TESTS)
  enable_testing()
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

add_subdirectory(modules/shared_helper)
add_subdirectory(modules/dispenser_service)
add_subdirectory(modules/rfid_service)
add_subdirectory(modules/cloud_service)
add_subdirectory(modules/vending_engine_service)

if(VENDING_BUILD_TESTS)
  add_subdirectory(modules/shared_helper/tests)
endif()

# ui_service: Qt/QML shell, opt-in (requires Qt6 installed). vending_app is
# the composition root (root main.cpp) that wires every module together and
# links ui_service as its Qt/QML facade.
if(VENDING_BUILD_APP)
  add_subdirectory(modules/ui_service)

  find_package(Qt6 COMPONENTS Core Gui Qml Quick QuickControls2 REQUIRED)
  set(CMAKE_AUTOMOC ON)

  qt_add_executable(vending_app main.cpp)

  target_link_libraries(vending_app PRIVATE
    vending::ui_service
    vending::vending_engine_service
    vending::cloud_service
    vending::cloud_service::rest_transport
    vending::dispenser_service::dummy
    vending::rfid_service::dummy
    vending::shared_helper
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
  )
endif()

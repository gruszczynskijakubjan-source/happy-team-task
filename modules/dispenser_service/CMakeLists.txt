add_library(dispenser_service INTERFACE)

target_include_directories(dispenser_service INTERFACE ${CMAKE_SOURCE_DIR}/modules)
target_compile_features(dispenser_service INTERFACE cxx_std_20)

add_library(vending::dispenser_service ALIAS dispenser_service)

add_library(dispenser_service_dummy STATIC
  impl/dummy_dispenser.cpp
)

target_include_directories(dispenser_service_dummy PUBLIC ${CMAKE_SOURCE_DIR}/modules)
target_link_libraries(dispenser_service_dummy PUBLIC dispenser_service vending::shared_helper)
target_compile_features(dispenser_service_dummy PUBLIC cxx_std_20)

add_library(vending::dispenser_service::dummy ALIAS dispenser_service_dummy)

add_library(dispenser_service_mocks INTERFACE)

target_include_directories(dispenser_service_mocks INTERFACE ${CMAKE_SOURCE_DIR}/modules)
target_link_libraries(dispenser_service_mocks INTERFACE dispenser_service GTest::gmock)

add_library(vending::dispenser_service::mocks ALIAS dispenser_service_mocks)

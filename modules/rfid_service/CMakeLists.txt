add_library(rfid_service INTERFACE)

target_include_directories(rfid_service INTERFACE ${CMAKE_SOURCE_DIR}/modules)
target_compile_features(rfid_service INTERFACE cxx_std_20)

add_library(vending::rfid_service ALIAS rfid_service)

add_library(rfid_service_dummy STATIC
  impl/dummy_card_reader.cpp
)

target_include_directories(rfid_service_dummy PUBLIC ${CMAKE_SOURCE_DIR}/modules)
target_link_libraries(rfid_service_dummy PUBLIC rfid_service vending::shared_helper)
target_compile_features(rfid_service_dummy PUBLIC cxx_std_20)

add_library(vending::rfid_service::dummy ALIAS rfid_service_dummy)

add_library(rfid_service_mocks INTERFACE)

target_include_directories(rfid_service_mocks INTERFACE ${CMAKE_SOURCE_DIR}/modules)
target_link_libraries(rfid_service_mocks INTERFACE rfid_service GTest::gmock)

add_library(vending::rfid_service::mocks ALIAS rfid_service_mocks)

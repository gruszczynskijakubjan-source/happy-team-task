add_library(vending_engine_service STATIC
  impl/state_machine.cpp
  impl/transaction_service.cpp
)

target_include_directories(vending_engine_service PUBLIC ${CMAKE_SOURCE_DIR}/modules)

target_link_libraries(vending_engine_service PUBLIC
  vending::shared_helper
  vending::cloud_service
  vending::dispenser_service
  vending::rfid_service
)

add_library(vending::vending_engine_service ALIAS vending_engine_service)


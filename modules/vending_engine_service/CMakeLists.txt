add_library(vending_engine_service STATIC
  impl/vending_engine_service.cpp
)

target_include_directories(vending_engine_service PUBLIC ${CMAKE_SOURCE_DIR}/modules)

target_link_libraries(vending_engine_service PUBLIC
  vending::shared_helper
  vending::cloud_service
  vending::dispenser_service
)

add_library(vending::vending_engine_service ALIAS vending_engine_service)


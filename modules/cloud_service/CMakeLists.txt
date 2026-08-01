add_library(cloud_service STATIC
  impl/cloud_service.cpp
  impl/sqlite_outbox_storage.cpp
)

target_include_directories(cloud_service PUBLIC ${CMAKE_SOURCE_DIR}/modules)

target_link_libraries(cloud_service PUBLIC
  vending::shared_helper
)

target_compile_features(cloud_service PUBLIC cxx_std_20)

add_library(vending::cloud_service ALIAS cloud_service)

find_package(Boost REQUIRED COMPONENTS system)

add_library(cloud_service_rest_transport STATIC
  impl/rest_transport.cpp
)

target_include_directories(cloud_service_rest_transport PUBLIC ${CMAKE_SOURCE_DIR}/modules)

target_link_libraries(cloud_service_rest_transport PUBLIC
  cloud_service
  Boost::system
)

target_compile_features(cloud_service_rest_transport PUBLIC cxx_std_20)

add_library(vending::cloud_service::rest_transport ALIAS cloud_service_rest_transport)

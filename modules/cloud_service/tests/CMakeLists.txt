add_executable(cloud_service_tests
  cloud_service_test.cpp
)

target_link_libraries(cloud_service_tests PRIVATE
  vending::cloud_service
  vending::cloud_service::mocks
  vending::shared_helper
  GTest::gtest
  GTest::gtest_main
  GTest::gmock
)

include(GoogleTest)
gtest_discover_tests(cloud_service_tests)

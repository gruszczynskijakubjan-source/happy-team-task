add_executable(vending_engine_service_tests
  vending_engine_service_test.cpp
)

target_link_libraries(vending_engine_service_tests PRIVATE
  vending::vending_engine_service
  vending::cloud_service::mocks
  vending::dispenser_service::mocks
  vending::shared_helper::mocks
  GTest::gtest
  GTest::gtest_main
  GTest::gmock
)

include(GoogleTest)
gtest_discover_tests(vending_engine_service_tests)

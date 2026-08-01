file(GLOB shared_helper_test_sources CONFIGURE_DEPENDS "*.cpp")

if(shared_helper_test_sources)
  add_executable(shared_helper_tests ${shared_helper_test_sources})

  target_link_libraries(shared_helper_tests PRIVATE
    vending::shared_helper
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
  )

  include(GoogleTest)
  gtest_discover_tests(shared_helper_tests)
endif()

find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED)

include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(shared_helper STATIC
  impl/random_uuid_generator.cpp
  impl/system_clock.cpp
  persistence/json_machine_state_store.cpp
  sql/sqlite_sql_adapter.cpp
  logging/console_logger.cpp
)

target_include_directories(shared_helper PUBLIC ${CMAKE_SOURCE_DIR}/modules)

target_link_libraries(shared_helper
  PUBLIC nlohmann_json::nlohmann_json
  PRIVATE SQLite::SQLite3
  PUBLIC Threads::Threads
  PRIVATE Boost::boost
)

target_compile_features(shared_helper PUBLIC cxx_std_20)

add_library(vending::shared_helper ALIAS shared_helper)

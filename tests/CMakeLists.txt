find_package(Python REQUIRED COMPONENTS
  Development.Embed # needed to embed Python::Python in C++ unit tests
)

add_custom_target(copy-fixtures ALL
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/__fixtures__"
  "${CMAKE_CURRENT_BINARY_DIR}/__fixtures__"
)

add_executable(test_chrono
  test_chrono.cpp
)
target_link_libraries(test_chrono
  PRIVATE
    GTest::gtest_main
    spdlog::spdlog_header_only
)

add_executable(test_irimager_class
  test_irimager_class.cpp
)
target_link_libraries(test_irimager_class
  PRIVATE
    GTest::gtest
    Python::Python
    irimager_class
)

add_executable(test_irlogger_parser
  test_irlogger_parser.cpp
)
target_link_libraries(test_irlogger_parser
  PRIVATE
    GTest::gtest
    gmock_main
    irlogger_parser
)

add_executable(test_string_ring_buffer
  test_string_ring_buffer.cpp
)
target_link_libraries(test_string_ring_buffer
  PRIVATE
    GTest::gtest_main
)

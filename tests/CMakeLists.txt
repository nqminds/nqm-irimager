find_package(Python REQUIRED COMPONENTS
  Development.Embed # needed to embed Python::Python in C++ unit tests
)

add_custom_target(copy-fixtures ALL
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/__fixtures__"
  "${CMAKE_CURRENT_BINARY_DIR}/__fixtures__"
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

cmake_minimum_required(VERSION 3.15...3.26)

# parse just the main numeric part of a PEP 440 version
string(REGEX MATCH
    [=[
        [0-9]+(\.[0-9]+(\.[0-9]+(\.[0-9]+)?)?)?
    ]=]
    skbuild_project_numeric_version
    "${SKBUILD_PROJECT_VERSION}"
)

if (NOT SKBUILD)
  message(WARNING "\
  This CMake file is meant to be executed using 'scikit-build-core'. You should
  probably be running this CMake file by using a command like `pip install .`.
  ")
endif()

project(
  "${SKBUILD_PROJECT_NAME}"
  VERSION "${skbuild_project_numeric_version}"
  LANGUAGES CXX)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/docstrings.h"
  COMMAND Python::Interpreter
  ARGS
    -m pybind11_mkdoc
    --output "${CMAKE_CURRENT_BINARY_DIR}/docstrings.h"
    "-I;$<JOIN:$<TARGET_PROPERTY:irimager,INCLUDE_DIRECTORIES>,;-I;>"
    -std=c++17
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nqm/irimager/irimager_class.hpp"
  COMMAND_EXPAND_LISTS
  VERBATIM
)

python_add_library(irimager MODULE
  src/nqm/irimager/irimager.cpp
  src/nqm/irimager/irimager_class.cpp
  "${CMAKE_CURRENT_BINARY_DIR}/docstrings.h"
  WITH_SOABI
)
set_target_properties(irimager PROPERTIES
  PRIVATE_HEADER
    "src/nqm/irimager/irimager_class.hpp;${CMAKE_CURRENT_BINARY_DIR}/docstrings.h"
)
target_link_libraries(irimager PRIVATE pybind11::headers)
target_compile_features(irimager PRIVATE cxx_std_17)
target_compile_definitions(irimager PRIVATE
  VERSION_INFO=${PROJECT_VERSION}
  DOCSTRINGS_H="${CMAKE_CURRENT_BINARY_DIR}/docstrings.h"
)

install(TARGETS irimager DESTINATION "nqm")

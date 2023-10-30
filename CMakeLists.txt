cmake_minimum_required(VERSION 3.25...3.26)

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

set(CMAKE_CXX_STANDARD 17) # we use C++17 std::string_view
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# CMake Package Manager
include(CPM)
CPMUsePackageLock(package-lock.cmake)

CPMGetPackage(googletest)
CPMGetPackage(spdlog)
CPMGetPackage(propagate_const)
CPMGetPackage(eigen)

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
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nqm/irimager/logger.hpp"
  COMMAND_EXPAND_LISTS
  VERBATIM
)

set(IRImager_mock OFF CACHE BOOL "If set, use a mock IRImager implementation that mocks an OPTIS IR camera")

add_compile_options(
  # Error on all C/C++ warnings if making a Debug build
  $<$<CONFIG:Debug>:-Werror>
  -Wall
  -Wextra
  -Wpedantic
  -Wconversion
  -Wsign-conversion
  -Wcast-qual
  -Wformat=2
  -Wundef
  -Werror=float-equal
  -Wshadow
  -Wcast-align
  -Wunused
  -Wnull-dereference
  -Wdouble-promotion
  -Wimplicit-fallthrough
  $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
  $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>
  $<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>
  $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>
)

add_library(irimager_class OBJECT
  "src/nqm/irimager/irimager_class.cpp"
)
set_target_properties(irimager_class PROPERTIES
  PRIVATE_HEADER
    "src/nqm/irimager/irimager_class.hpp"
  POSITION_INDEPENDENT_CODE ON # -fPIC
)
target_link_libraries(irimager_class
  PUBLIC
    propagate_const::propagate_const
    Eigen3::Eigen
  PRIVATE
    spdlog::spdlog_header_only # less efficient, but avoids CXX11 ABI issues
)

if(IRImager_mock)
  target_compile_definitions(irimager_class PRIVATE IR_IMAGER_MOCK)
else()
  find_package(IRImager 4.0...5.0)

  if(NOT IRImager_FOUND)
    message(FATAL_ERROR "libirimager was not found. \
      It can be downloaded from https://evocortex.org/products/irimagerdirect-sdk/. \
      Alternatively, compile with `SKBUILD_CMAKE_DEFINE='IRImager_mock=ON'` to
      create a mocked extension \
      that doesn't need libirimager, or a thermal camera.")
  endif()

  target_link_libraries(irimager_class PRIVATE IRImager::IRImager)
endif(IRImager_mock)

add_library(irlogger_parser OBJECT
  "src/nqm/irimager/irlogger_parser.cpp"
)
target_compile_definitions(irlogger_parser PRIVATE "${IRImager_DEFINITIONS}")
set_target_properties(irlogger_parser PROPERTIES
  PRIVATE_HEADER
    "src/nqm/irimager/irlogger_parser.hpp"
  POSITION_INDEPENDENT_CODE ON # -fPIC
)

if (UNIX)
  add_library(irlogger_to_spd OBJECT
    "src/nqm/irimager/irlogger_to_spd_posix.cpp"
  )
else()
  message(FATAL_ERROR "The nqm.irimager package only currently supports POSIX/UNIX systems.")
endif()

set_target_properties(irlogger_to_spd PROPERTIES
  PRIVATE_HEADER
    "src/nqm/irimager/irlogger_to_spd.hpp"
  POSITION_INDEPENDENT_CODE ON # -fPIC
)
target_link_libraries(irlogger_to_spd
  PUBLIC
    propagate_const::propagate_const
  PRIVATE
    pybind11::pybind11
    spdlog::spdlog_header_only
    irlogger_parser
)

if(IRImager_mock)
  target_compile_definitions(irlogger_to_spd PRIVATE IR_IMAGER_MOCK)
elseif()
  target_link_libraries(irlogger_to_spd PRIVATE IRImager::IRCore)
endif()

add_library(logger OBJECT
  "src/nqm/irimager/logger.cpp"
)
set_target_properties(logger PROPERTIES
  PRIVATE_HEADER
    "src/nqm/irimager/logger.hpp"
  POSITION_INDEPENDENT_CODE ON # -fPIC
)
target_compile_definitions(logger PRIVATE "${IRImager_DEFINITIONS}")
target_link_libraries(logger
  PUBLIC
    pybind11::pybind11
  PRIVATE
    spdlog::spdlog_header_only
    irlogger_to_spd
)

pybind11_add_module(irimager MODULE
  "src/nqm/irimager/irimager.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/docstrings.h"
)
set_target_properties(irimager PROPERTIES
  PRIVATE_HEADER
    "${CMAKE_CURRENT_BINARY_DIR}/docstrings.h"
)
target_compile_definitions(irimager PRIVATE "${IRImager_DEFINITIONS}")
target_link_libraries(irimager
  PRIVATE
    pybind11::headers
    irimager_class
    irlogger_parser
    irlogger_to_spd
    logger
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  # C++ unit tests can be run using CTest
  # they can also by run with pytest
  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(tests)
  endif()
endif()

target_compile_definitions(irimager PRIVATE
  DOCSTRINGS_H="${CMAKE_CURRENT_BINARY_DIR}/docstrings.h"
  SKBUILD_PROJECT_VERSION="${SKBUILD_PROJECT_VERSION}"
)

install(TARGETS irimager DESTINATION "nqm")

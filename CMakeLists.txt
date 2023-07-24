cmake_minimum_required(VERSION 3.15...3.26)

# parse just the main numeric part of a PEP 440 version
string(REGEX MATCH
    [=[
        [0-9]+(\.[0-9]+(\.[0-9]+(\.[0-9]+)?)?)?
    ]=]
    skbuild_project_numeric_version
    "${SKBUILD_PROJECT_VERSION}"
)

project(
  ${SKBUILD_PROJECT_NAME}
  VERSION "${skbuild_project_numeric_version}"
  LANGUAGES CXX)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

python_add_library(irimager MODULE src/nqm/irimager/irimager.cpp WITH_SOABI)
target_link_libraries(irimager PRIVATE pybind11::headers)
target_compile_definitions(irimager PRIVATE VERSION_INFO=${PROJECT_VERSION})

install(TARGETS irimager DESTINATION "nqm")

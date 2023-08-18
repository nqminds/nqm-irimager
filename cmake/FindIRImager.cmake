# SPDX-FileCopyrightText: © 2023 Nquiringminds Ltd

#[=======================================================================[.rst:
FindIRImager
------------

Find IRImager include directory and library.

Imported Targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``IRImager::IRImager``
  The `libirimager` library, if found.
``IRImager::IRImageProcessing``
  The `libirimageprocessing` library, if found.
``IRImager::IRCore``
  The `libircore` library, if found.

An :ref:`imported target <Imported targets>` named
 is provided if IRImager has been found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``IRImager_FOUND``
  True if IRImager was found, false otherwise.
``IRImager_INCLUDE_DIRS``
  Include directories needed to include IRImager headers.
``IRImager_LIBRARIES``
  Libraries needed to link to IRImager.
``IRImager_DEFINITIONS``
  Preprocessor definitions needed to compile/link to IRImager properly.
  IRImager may require using `_GLIBCXX_USE_CXX11_ABI=0`.

Cache Variables
^^^^^^^^^^^^^^^

This module uses the following cache variables:

``IRImager_LIBRARY``
  The location of the IRImager library file.
``IRImager_IRImageProcessing_LIBRARY``
  The location of the IRImageProcessing library file.
``IRImager_IRCore_LIBRARY``
  The location of the IRCore library file.
``IRImager_INCLUDE_DIR``
  The location of the IRImager include directory containing `libirimager/IR*.h`.
``IRImager_VERSION``
  The version of the IRImager library. This requires compiling and running
  a small test program, so this value will not be set when cross-compiling.

The cache variables should not be used by project code.
They may be set by end users to point at IRImager components.
#]=======================================================================]

find_package(UDev REQUIRED)
find_package(LibUsb REQUIRED)

#-----------------------------------------------------------------------------
find_library(IRImager_LIBRARY
  NAMES irimager
)
mark_as_advanced(IRImager_LIBRARY)

find_path(IRImager_INCLUDE_DIR
  NAMES libirimager/IRDevice.h
)
mark_as_advanced(IRImager_INCLUDE_DIR)

find_library(IRImager_IRImageProcessing_LIBRARY
  NAMES irimageprocessing
)
mark_as_advanced(IRImager_IRImageProcessing_LIBRARY)

find_library(IRImager_IRCore_LIBRARY
  NAMES ircore
)
mark_as_advanced(IRImager_IRCore_LIBRARY)

#-----------------------------------------------------------------------------
#---       Try to find libirimager version by calling getVersion()         ---

if(IRImager_IRCore_LIBRARY AND IRImager_IRImageProcessing_LIBRARY AND IRImager_LIBRARY AND IRImager_INCLUDE_DIR)
  file(
    WRITE
    "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/get-irimager-version.cpp"
    [=[
    #include <iostream>
    #include <libirimager/IRImager.h>

    int main() {
      std::cout << evo::IRImager::getVersion();
    }
    ]=]
  )

  # won't run when cross-compiling without an emulator
  try_run(
    _IRImager_getVersion_run_result
    _IRImager_getVersion_compile_result
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/get-irimager-version.cpp"
    COMPILE_DEFINITIONS "-D_GLIBCXX_USE_CXX11_ABI=0"
    LINK_LIBRARIES
      "${IRImager_LIBRARY}"
      "${IRImager_IRImageProcessing_LIBRARY}"
      "${IRImager_IRCore_LIBRARY}"
      UDev::UDev
      LibUsb::LibUsb
    CMAKE_FLAGS
      "-DINCLUDE_DIRECTORIES:STRING=${IRImager_INCLUDE_DIR}"
    RUN_OUTPUT_VARIABLE IRImager_VERSION
  )
endif()

#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(IRImager
  FOUND_VAR IRImager_FOUND
  REQUIRED_VARS
    IRImager_LIBRARY
    IRImager_INCLUDE_DIR
    IRImager_IRImageProcessing_LIBRARY
    IRImager_IRCore_LIBRARY
  VERSION_VAR
    IRImager_VERSION
  )
set(IRImager_FOUND "${IRImager_FOUND}")

# Newer versions of IRImager might need this to be set to 1,
# at least according to http://ftp.evocortex.com/sdk_compiler_options.html
set(IRImager_DEFINITIONS "_GLIBCXX_USE_CXX11_ABI=0")

#-----------------------------------------------------------------------------
# Provide documented result variables and targets.
if(IRImager_FOUND)
  if(NOT TARGET IRImager::IRCore)
    add_library(IRImager::IRCore UNKNOWN IMPORTED)
    set_target_properties(IRImager::IRCore PROPERTIES
        IMPORTED_LOCATION "${IRImager_IRCore_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${IRImager_INCLUDE_DIRS}"
    )
    target_compile_definitions(IRImager::IRCore
      INTERFACE
        "${IRImager_DEFINITIONS}"
    )
  endif()

  if(NOT TARGET IRImager::IRImageProcessing)
    add_library(IRImager::IRImageProcessing UNKNOWN IMPORTED)
    set_target_properties(IRImager::IRImageProcessing PROPERTIES
      IMPORTED_LOCATION "${IRImager_IRImageProcessing_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${IRImager_INCLUDE_DIRS}"
    )
    # Needed despite `libirimageprocessing.so` not trying to link to other shared libs
    target_link_libraries(IRImager::IRImageProcessing
        INTERFACE
            IRImager::IRCore
            UDev::UDev
    )
  endif()

  if(NOT TARGET IRImager::IRImager)
    add_library(IRImager::IRImager UNKNOWN IMPORTED)
    set_target_properties(IRImager::IRImager PROPERTIES
      IMPORTED_LOCATION "${IRImager_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${IRImager_INCLUDE_DIRS}"
    )
    # Needed despite `libirimager.so` not trying to link to other shared libs
    target_link_libraries(IRImager::IRImager
        INTERFACE
            IRImager::IRImageProcessing
            IRImager::IRCore
            UDev::UDev
            LibUsb::LibUsb
    )
  endif()

  set(IRImager_INCLUDE_DIRS "${IRImager_INCLUDE_DIR}")
  set(IRImager_LIBRARIES "$<TARGET_PROPERTY:IRImager::IRImager,INTERFACE_LINK_LIBRARIES>")
endif()

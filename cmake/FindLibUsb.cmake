# SPDX-FileCopyrightText: © 2023 Nquiringminds Ltd

#[=======================================================================[.rst:
FindLibUsb
-----------

Find LibUsb-1.0, the Linux userspace USB programming library.

Imported Targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``LibUsb::LibUsb``
  The LibUsb ``usb`` library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``LibUsb_FOUND``
  True if LibUsb was found, false otherwise.
``LibUsb_INCLUDE_DIRS``
  LibUSB include directories.
``LibUsb_LIBRARIES``
  LibUSB library, and it's dependencies.
``LibUsb_VERSION``
  The version of LibUsb.

#]=======================================================================]
cmake_minimum_required(VERSION 3.15...3.26)

find_package(PkgConfig QUIET)
pkg_check_modules(PC_LibUsb QUIET libusb-1.0 IMPORTED_TARGET)

find_path(LibUsb_INCLUDE_DIRS NAMES libusb-1.0/libusb.h usb.h HINTS "${PC_LibUsb_INCLUDE_DIRS}")
find_library(LibUsb_LIBRARIES NAMES usb-1.0 HINTS "${PC_LibUsb_LIBRARY_DIRS}")

set(LibUsb_VERSION "${PC_LibUsb_VERSION}")

#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibUsb
  FOUND_VAR LibUsb_FOUND
  REQUIRED_VARS
    LibUsb_INCLUDE_DIRS LibUsb_LIBRARIES
  VERSION_VAR
    LibUsb_VERSION
)
#-----------------------------------------------------------------------------
if(LibUsb_FOUND)
  if(NOT TARGET LibUsb::LibUsb)
    add_library(LibUsb::LibUsb UNKNOWN IMPORTED)
    set_target_properties(LibUsb::LibUsb PROPERTIES
      IMPORTED_LOCATION "${LibUsb_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${LibUsb_INCLUDE_DIRS}"
      INTERFACE_COMPILE_DEFINITIONS "${PC_LibUsb_CFLAGS_OTHER}"
    )
  endif()
endif(LibUsb_FOUND)

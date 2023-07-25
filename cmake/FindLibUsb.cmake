# SPDX-FileCopyrightText: © 2023 Nquiringminds Ltd

#[=======================================================================[.rst:
FindLibUsb
-----------

Find LibUsb, the Linux userspace USB programming library.

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
``LibUsb_LIBRARIES``
  LibUSB library, and it's dependencies.
``LibUsb_VERSION``
  The version of LibUsb.

#]=======================================================================]
cmake_minimum_required(VERSION 3.15...3.26)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LibUsb REQUIRED libusb IMPORTED_TARGET)

if(_LibUsb_FOUND)
  set(LibUsb_FOUND "${_LibUsb_FOUND}")
  set(LibUsb_LIBRARIES "${_LibUsb_LINK_LIBRARIES}")
  set(LibUsb_VERSION "${_LibUsb_VERSION}")
endif(_LibUsb_FOUND)

#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibUsb
  FOUND_VAR LibUsb_FOUND
  REQUIRED_VARS
    LibUsb_LIBRARIES
  VERSION_VAR
    LibUsb_VERSION
)
#-----------------------------------------------------------------------------
if(LibUsb_FOUND)
  if(NOT TARGET LibUsb::LibUsb)
    add_library(LibUsb::LibUsb INTERFACE IMPORTED)
    target_link_libraries(LibUsb::LibUsb INTERFACE PkgConfig::LibUsb)
  endif()
endif(LibUsb_FOUND)

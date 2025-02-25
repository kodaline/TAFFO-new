# Copyright 2017 Politecnico di Milano.
# Developed by : Stefano Cherubin, Davide Gadioli
#                PhD student, Politecnico di Milano
#                <first_name>.<family_name>@polimi.it
#
# This file is part of libVersioningCompiler
#
# libVersioningCompiler is free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation, either version 3
# of the License, or (at your option) any later version.
#
# libVersioningCompiler is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with libVersioningCompiler. If not, see <http://www.gnu.org/licenses/>

# - Check for the presence of libVersioningCompiler
#
# The following variables are set when libVersioningCompiler is found:
#  HAVE_LIBVC       = Set to true, if all components of libVersioningCompiler
#                          have been found.
#  LIBVC_INCLUDES   = Include path for the header files of libVersioningCompiler
#  LIBVC_LIBRARIES  = Link these libraries to use libVersioningCompiler
#  LIVC_LIB_DIR     = Extra libraries directories
#  HAVE_CLANG_LIB_COMPILER = Set to true if libVersioningCompiler can have
#                                 Clang as a library enabled

## -----------------------------------------------------------------------------
## Check for the header files

set(LIBVC_SOURCE_PATH /home/vagrant/libVersioningCompiler )
set(LIBVC_BINARY_PATH /home/vagrant/libVersioningCompiler/build )
set(LIBVC_INSTALL_PATH /usr/local )

# check for the header of the monitor module
find_path (LIBVC_INCLUDES versioningCompiler/Version.hpp PATHS
                    ${LIBVC_INSTALL_PATH}/include
                    ${LIBVC_SOURCE_PATH}/include
                    /usr/local/include
                    /usr/include
                    ${CMAKE_EXTRA_INCLUDES}
          )

if (LIBVC_INCLUDES-NOTFOUND)
  set (HAVE_LIBVC FALSE)
  if (LibVersioningCompiler_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find libVersioningCompiler")
  endif (LibVersioningCompiler_FIND_REQUIRED)
endif (LIBVC_INCLUDES-NOTFOUND)

# Check for Clang-as-a-library implementation is installed
find_file (CLANG_LIB_COMPILER_INCLUDE_PATH
           versioningCompiler/CompilerImpl/ClangLibCompiler.hpp
           PATHS ${LIBVC_INCLUDES}
           NO_DEFAULT_PATH
          )

if (CLANG_LIB_COMPILER_INCLUDE_PATH-NOTFOUND)
  set (HAVE_CLANG_LIB_COMPILER FALSE)
else (CLANG_LIB_COMPILER_INCLUDE_PATH-NOTFOUND)
  set (HAVE_CLANG_LIB_COMPILER TRUE)
endif (CLANG_LIB_COMPILER_INCLUDE_PATH-NOTFOUND)

## -----------------------------------------------------------------------------
## Check for DEPENDENCIES headers

## -----------------------------------------------------------------------------
## Check for DL includes

set(DL_ROOT $ENV{DL_ROOT})

find_path (DL_INCLUDES dlfcn.h
  PATHS ${DL_ROOT}/include
  NO_DEFAULT_PATH
  )

if (NOT DL_INCLUDES)
  find_path (DL_INCLUDES dlfcn.h
    PATHS /usr/local/include /usr/include ${CMAKE_EXTRA_INCLUDES}
    )
endif (NOT DL_INCLUDES)

if (DL_INCLUDES-NOTFOUND)
  set (HAVE_LIBVC FALSE)
  if (LibVersioningCompiler_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find libVersioningCompiler required dependency DL")
  endif (LibVersioningCompiler_FIND_REQUIRED)
endif (DL_INCLUDES-NOTFOUND)

## -----------------------------------------------------------------------------
## Check for UUID includes

set(UUID_ROOT $ENV{UUID_ROOT})

if (APPLE)
  set(UUID_LIBRARY_VAR System)
else (APPLE)
  # Linux type:
  SET(UUID_LIBRARY_VAR uuid)
endif (APPLE)

find_path (UUID_INCLUDE_DIR
  uuid/uuid.h
  PATHS ${UUID_ROOT}/lib
  NO_DEFAULT_PATH
  )

if (NOT UUID_INCLUDE_DIR)
  FIND_PATH(UUID_INCLUDE_DIR uuid/uuid.h
    /usr/local/include
    /usr/include
  )
endif (NOT UUID_INCLUDE_DIR)

if (UUID_INCLUDE_DIR-NOTFOUND)
  set (HAVE_LIBVC FALSE)
  if (LibVersioningCompiler_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find libVersioningCompiler required dependency UUID")
  endif (LibVersioningCompiler_FIND_REQUIRED)
endif (UUID_INCLUDE_DIR-NOTFOUND)

## -----------------------------------------------------------------------------
# Find the native LLVM includes and library
#
#  LLVM_INCLUDE_DIR - where to find llvm include files
#  LLVM_LIBRARY_DIR - where to find llvm libs
#  LLVM_CFLAGS      - llvm compiler flags
#  LLVM_LFLAGS      - llvm linker flags
#  LLVM_MODULE_LIBS - list of llvm libs for working with modules.
#  LLVM_FOUND       - True if llvm found.
set (LIBCLANG_KNOWN_LLVM_VERSIONS 8.0.0 8.0 8
  7.0.1 7.0.0 7.0 7
  6.0.1 6.0.0 6.0 6
  5.0.2 5.0.1 5.0.0 5.0 5
  4.0.1 4.0.0 4.0 4
  3.9.1 3.9.0 3.9
  3.8.1 3.8.0 3.8
  3.7.1 3.7.0 3.7
  3.6.2 3.6.1 3.6.0 3.6
  3.5.2 3.5.1 3.5.0 3.5
  3.4.2 3.4.1 3.4
  3.3
  3.2
  3.1)

set (llvm_config_name)
set (llvm_header_search_paths)
set (llvm_lib_search_paths
                            # LLVM Fedora
                            /usr/lib/llvm
                          )

foreach (version ${LIBCLANG_KNOWN_LLVM_VERSIONS})
  string(REPLACE "." "" undotted_version "${version}")
  list(APPEND llvm_header_search_paths
    # LLVM Debian/Ubuntu nightly packages: http://apt.llvm.org/
    "/usr/lib/llvm-${version}/include/"
    # LLVM MacPorts
    "/opt/local/libexec/llvm-${version}/include"
    # LLVM Homebrew
    "/usr/local/Cellar/llvm/${version}/include"
    # LLVM Homebrew/versions
    "/usr/local/lib/llvm-${version}/include"
    # FreeBSD ports versions
    "/usr/local/llvm${undotted_version}/include"
    )

  list(APPEND llvm_config_name
                              llvm-config-${version}
                              llvm-config${version}
                              llvm-config-${undotted_version}
                              llvm-config${undotted_version}
                              llvm-config
                            )

  list(APPEND llvm_lib_search_paths
    # LLVM Debian/Ubuntu nightly packages: http://apt.llvm.org/
    "/usr/lib/llvm-${version}/lib/"
    # LLVM MacPorts
    "/opt/local/libexec/llvm-${version}/lib"
    # LLVM Homebrew
    "/usr/local/Cellar/llvm/${version}/lib"
    # LLVM Homebrew/versions
    "/usr/local/lib/llvm-${version}/lib"
    # FreeBSD ports versions
    "/usr/local/llvm${undotted_version}/lib"
    )
endforeach()

find_program(LLVM_CONFIG_EXECUTABLE
             NAMES ${llvm_config_name}
             DOC "llvm-config executable")

if (LLVM_CONFIG_EXECUTABLE)
  set (HAVE_LLVM TRUE)
  if (NOT LibVersioningCompiler_FIND_QUIETLY)
    message (STATUS "Found components for LLVM")
    message (STATUS "llvm-config ........ = ${LLVM_CONFIG_EXECUTABLE}")
  endif (NOT LibVersioningCompiler_FIND_QUIETLY)
else (LLVM_CONFIG_EXECUTABLE)
  set (HAVE_LLVM FALSE)
  if (NOT LibVersioningCompiler_FIND_QUIETLY)
    message (STATUS "Unable to find llvm-config executable")
    message (WARNING "Could NOT find LLVM config executable")
  endif (NOT LibVersioningCompiler_FIND_QUIETLY)
endif (LLVM_CONFIG_EXECUTABLE)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
  OUTPUT_VARIABLE LLVM_INCLUDE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir
  OUTPUT_VARIABLE LLVM_LIBRARY_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --cppflags
  OUTPUT_VARIABLE LLVM_CFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
  OUTPUT_VARIABLE LLVM_LFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs
  OUTPUT_VARIABLE LLVM_MODULE_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --version
  OUTPUT_VARIABLE LLVM_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

## -----------------------------------------------------------------------------
# Try to find libclang
#
# Once done this will define:
# - HAVE_LIBCLANG
#               System has libclang.
# - LIBCLANG_INCLUDES
#               The libclang include directories.
# - LIBCLANG_LIBRARIES
#               The libraries needed to use libclang.
# - LIBCLANG_LIBRARY_DIR
#               The path to the directory containing libclang.
# - LIBCLANG_KNOWN_LLVM_VERSIONS
#               Known LLVM release numbers.

# most recent versions come first
# http://apt.llvm.org/
set (LIBCLANG_KNOWN_LLVM_VERSIONS 8.0.0 8.0 8
  7.0.1 7.0.0 7.0 7
  6.0.1 6.0.0 6.0 6
  5.0.2 5.0.1 5.0.0 5.0 5
  4.0.1 4.0.0 4.0 4
  3.9.1 3.9.0 3.9
  3.8.1 3.8.0 3.8
  3.7.1 3.7.0 3.7
  3.6.2 3.6.1 3.6.0 3.6
  3.5.2 3.5.1 3.5.0 3.5
  3.4.2 3.4.1 3.4
  3.3
  3.2
  3.1)

set (libclang_llvm_header_search_paths)
set (libclang_llvm_lib_search_paths
                                  # LLVM Fedora
                                  /usr/lib/llvm
                                )

foreach (version ${LIBCLANG_KNOWN_LLVM_VERSIONS})
  string(REPLACE "." "" undotted_version "${version}")
  list(APPEND libclang_llvm_header_search_paths
    # LLVM Debian/Ubuntu nightly packages: http://apt.llvm.org/
    "/usr/lib/llvm-${version}/include/"
    # LLVM MacPorts
    "/opt/local/libexec/llvm-${version}/include"
    # LLVM Homebrew
    "/usr/local/Cellar/llvm/${version}/include"
    # LLVM Homebrew/versions
    "/usr/local/lib/llvm-${version}/include"
    # FreeBSD ports versions
    "/usr/local/llvm${undotted_version}/include"
    )

  list(APPEND libclang_llvm_lib_search_paths
    # LLVM Debian/Ubuntu nightly packages: http://apt.llvm.org/
    "/usr/lib/llvm-${version}/lib/"
    # LLVM MacPorts
    "/opt/local/libexec/llvm-${version}/lib"
    # LLVM Homebrew
    "/usr/local/Cellar/llvm/${version}/lib"
    # LLVM Homebrew/versions
    "/usr/local/lib/llvm-${version}/lib"
    # FreeBSD ports versions
    "/usr/local/llvm${undotted_version}/lib"
    )
endforeach()

find_path (LIBCLANG_INCLUDES clang-c/Index.h
  PATHS ${libclang_llvm_header_search_paths}
  PATH_SUFFIXES LLVM/include #Windows package from http://llvm.org/releases/
  DOC "The path to the directory that contains clang-c/Index.h")

if (LIBCLANG_INCLUDES)
  set (HAVE_LIBCLANG TRUE)
else(LIBCLANG_INCLUDES)
  set (HAVE_LIBCLANG FALSE)
endif(LIBCLANG_INCLUDES)

## -----------------------------------------------------------------------------
# Check LLVM version

set (LLVM_VERSION_MIN_REQUIRED "4.0.0")

if ( HAVE_LLVM AND
     (NOT ( LLVM_VERSION VERSION_LESS LLVM_VERSION_MIN_REQUIRED )) AND
     HAVE_LIBCLANG )
  set ( ENABLE_CLANG_AS_LIB TRUE )
  if (NOT HAVE_CLANG_LIB_COMPILER AND
      NOT LibVersioningCompiler_FIND_QUIETLY)
    message( WARNING "Clang-as-a-library can be enabled\
 however it is not installed.\n\
 Please re-install libVersioningCompiler to enable it" )
  endif (NOT HAVE_CLANG_LIB_COMPILER AND
         NOT LibVersioningCompiler_FIND_QUIETLY)
else()
  set ( ENABLE_CLANG_AS_LIB FALSE )
  if (NOT LibVersioningCompiler_FIND_QUIETLY)
    if (NOT HAVE_LLVM)
      message( WARNING "LLVM not detected" )
    else(NOT HAVE_LLVM)
      if (LLVM_VERSION VERSION_LESS LLVM_VERSION_MIN_REQUIRED)
        message( WARNING "You are using an obsolete version of LLVM:\
  \n\t${LLVM_VERSION} detected.\t\
  Minimum required is ${LLVM_VERSION_MIN_REQUIRED}" )
      else(LLVM_VERSION VERSION_LESS LLVM_VERSION_MIN_REQUIRED)
        message( WARNING "LibClang not detected" )
      endif (LLVM_VERSION VERSION_LESS LLVM_VERSION_MIN_REQUIRED)
    endif (NOT HAVE_LLVM)
    message( WARNING "Clang-as-a-library disabled" )
  endif (NOT LibVersioningCompiler_FIND_QUIETLY)
endif()

# Make variable coherent
if (HAVE_CLANG_LIB_COMPILER AND NOT ENABLE_CLANG_AS_LIB)
  set (HAVE_CLANG_LIB_COMPILER FALSE)
endif (HAVE_CLANG_LIB_COMPILER AND NOT ENABLE_CLANG_AS_LIB)
if (ENABLE_CLANG_AS_LIB AND NOT HAVE_CLANG_LIB_COMPILER)
  set (ENABLE_CLANG_AS_LIB FALSE)
endif (ENABLE_CLANG_AS_LIB AND NOT HAVE_CLANG_LIB_COMPILER)

## -----------------------------------------------------------------------------
# compose the real list of paths

set( LIBVC_INCLUDES ${LIBVC_INCLUDES}
                    ${DL_INCLUDES}
                    ${UUID_INCLUDE_DIR}
                  )

if (ENABLE_CLANG_AS_LIB)
  set( LIBVC_INCLUDES ${LIBVC_INCLUDES}
                      ${LLVM_INCLUDE_DIR}
                      ${LIBCLANG_INCLUDES}
                    )
endif(ENABLE_CLANG_AS_LIB)

list( REMOVE_DUPLICATES LIBVC_INCLUDES )

## -----------------------------------------------------------------------------
## -----------------------------------------------------------------------------
## Check for the libraries

# On Windows with MSVC, the import library uses the ".imp" file extension
# instead of the comon ".lib"
if (MSVC)
  find_file (LIBCLANG_LIBRARY libclang.imp
    PATH_SUFFIXES LLVM/lib
    DOC "The file that corresponds to the libclang library.")
endif(MSVC)

find_library (LIBCLANG_LIBRARY NAMES libclang.imp libclang clang
  PATHS ${libclang_llvm_lib_search_paths}
  PATH_SUFFIXES LLVM/lib #Windows package from http://llvm.org/releases/
  DOC "The file that corresponds to the libclang library.")

get_filename_component (LIBCLANG_LIBRARY_DIR ${LIBCLANG_LIBRARY} PATH)

MACRO(FIND_AND_ADD_CLANG_LIB _libname_)
find_library(CLANG_${_libname_}_LIB
  NAMES ${_libname_}
  PATHS ${libclang_llvm_lib_search_paths} ${LLVM_LIBRARY_DIR})
if (CLANG_${_libname_}_LIB)
   set (LIBCLANG_LIBRARIES ${LIBCLANG_LIBRARIES} ${CLANG_${_libname_}_LIB})
   get_filename_component (LIBCLANG_${_libname_}_LIBRARY_DIR ${LIBCLANG_LIBRARY} PATH)
   set (LIBCLANG_LIBRARY_DIR ${LIBCLANG_LIBRARY_DIR} ${LIBCLANG_${_libname_}_LIBRARY_DIR} )
endif (CLANG_${_libname_}_LIB)
ENDMACRO(FIND_AND_ADD_CLANG_LIB)

set (LIBCLANG_LIBRARIES ${LIBCLANG_LIBRARY})
# Clang shared library provides just the limited C interface, so it
# can not be used.  We look for the static libraries.
FIND_AND_ADD_CLANG_LIB(clangFrontend)
FIND_AND_ADD_CLANG_LIB(clangSerialization)
FIND_AND_ADD_CLANG_LIB(clangDriver)
FIND_AND_ADD_CLANG_LIB(clangCodeGen)
FIND_AND_ADD_CLANG_LIB(clangParse)
FIND_AND_ADD_CLANG_LIB(clangSema)
FIND_AND_ADD_CLANG_LIB(clangAnalysis)
FIND_AND_ADD_CLANG_LIB(clangAST)
FIND_AND_ADD_CLANG_LIB(clangEdit)
FIND_AND_ADD_CLANG_LIB(clangLex)
FIND_AND_ADD_CLANG_LIB(clangBasic)

list (REMOVE_DUPLICATES LIBCLANG_LIBRARY_DIR)

set (LIBCLANG_LIBRARIES ${LIBCLANG_LIBRARY} ${LIBCLANG_LIBRARIES})

## -----------------------------------------------------------------------------
## Check for the DL library

find_library (DL_LIBRARY dl
  PATHS ${DL_ROOT}/lib
  NO_DEFAULT_PATH
  )

if (NOT DL_LIBRARY)
  find_library (DL_LIBRARY dl
    PATHS /usr/local/lib /usr/lib /lib ${CMAKE_EXTRA_LIBRARIES}
    )
endif (NOT DL_LIBRARY)

get_filename_component (DL_LIBRARY_DIR ${DL_LIBRARY} PATH)

## -----------------------------------------------------------------------------
## Check for the uuid library

find_library (UUID_LIBRARY
  NAMES ${UUID_LIBRARY_VAR}
  PATHS ${UUID_ROOT}/lib
  NO_DEFAULT_PATH
  )

if (NOT UUID_LIBRARY)
  find_library (UUID_LIBRARY
    NAMES ${UUID_LIBRARY_VAR}
    PATHS /lib /usr/lib /usr/local/lib
    )
endif (NOT UUID_LIBRARY)

get_filename_component (UUID_LIBRARY_DIR ${UUID_LIBRARY} PATH)

## -----------------------------------------------------------------------------
# check for the libVersioningCompiler library
find_library (LIBVC_LIBRARY VersioningCompiler PATHS
                ${LIBVC_INSTALL_PATH}/lib
                ${LIBVC_BINARY_PATH}
                /usr/local/lib
                /usr/lib /lib
                ${CMAKE_EXTRA_LIBRARIES}
  )

get_filename_component (LIBVC_LIBRARY_DIR ${LIBVC_LIBRARY} PATH)

set( LIBVC_LIBRARIES ${LIBVC_LIBRARY}
                     ${UUID_LIBRARY}
                     ${DL_LIBRARY}
                   )

set ( LIVC_LIB_DIR   ${LIBVC_LIBRARY_DIR}
                     ${UUID_LIBRARY_DIR}
                     ${DL_LIBRARY_DIR}
                   )

if (ENABLE_CLANG_AS_LIB)
  set ( LIVC_LIB_DIR   ${LIVC_LIB_DIR}
                       ${LIBCLANG_LIBRARY_DIR}
                       ${LLVM_LIBRARY_DIR}
                     )

  set( LIBVC_LIBRARIES ${LIBVC_LIBRARIES}
                       ${LIBCLANG_LIBRARIES}
                       ${LLVM_MODULE_LIBS}
                     )
endif(ENABLE_CLANG_AS_LIB)

list( REMOVE_DUPLICATES LIVC_LIB_DIR )

list( REMOVE_DUPLICATES LIBVC_LIBRARIES )

## -----------------------------------------------------------------------------
## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if ( LIBVC_INCLUDES AND LIBVC_LIBRARIES )
  set ( HAVE_LIBVC TRUE )
else ( LIBVC_INCLUDES AND LIBVC_LIBRARIES )
  if ( NOT LibVersioningCompiler_FIND_QUIETLY )
    if ( NOT LIBVC_INCLUDES )
      message (STATUS "Unable to find libVC header files!")
    endif ( NOT LIBVC_INCLUDES )
    if ( NOT LIBVC_LIBRARIES )
      message (STATUS "Unable to find libVC library files!")
    endif ( NOT LIBVC_LIBRARIES )
  endif ( NOT LibVersioningCompiler_FIND_QUIETLY )
endif ( LIBVC_INCLUDES AND LIBVC_LIBRARIES )

if ( HAVE_LIBVC )
  if ( NOT LibVersioningCompiler_FIND_QUIETLY )
    message (STATUS "Found components for libVersioningCompiler")
    message (STATUS "LIBVC_INCLUDES .... = ${LIBVC_INCLUDES}")
    message (STATUS "LIBVC_LIBRARIES ... = ${LIBVC_LIBRARIES}")
  endif ( NOT LibVersioningCompiler_FIND_QUIETLY )
else ( HAVE_LIBVC )
  if ( LibVersioningCompiler_FIND_REQUIRED )
    message (FATAL_ERROR "Could not find libVersioningCompiler!")
  else ( LibVersioningCompiler_FIND_REQUIRED )
    if ( LibVersioningCompiler_FIND_QUIETLY )
      message (WARNING "Could not find libVersioningCompiler!")
    endif ( LibVersioningCompiler_FIND_QUIETLY )
  endif ( LibVersioningCompiler_FIND_REQUIRED )
endif ( HAVE_LIBVC )

# set also standard cmake variable names
if (HAVE_LIBVC)
  set (LibVersioningCompiler_FOUND TRUE)
  set (LibVersioningCompiler_INCLUDE_DIRS ${LIBVC_INCLUDES})
  set (LibVersioningCompiler_INCLUDES ${LIBVC_INCLUDES})
  set (LibVersioningCompiler_LIBRARIES ${LIBVC_LIBRARIES})
  set (LibVersioningCompiler_LIBS ${LIBVC_LIBRARIES})
else (HAVE_LIBVC)
  set (LibVersioningCompiler_FOUND FALSE)
endif (HAVE_LIBVC)

mark_as_advanced (
    HAVE_LIBVC
    LIBVC_INCLUDES
    LIBVC_LIBRARIES
    LIVC_LIB_DIR
    HAVE_CLANG_LIB_COMPILER
    LibVersioningCompiler_FOUND
    LibVersioningCompiler_INCLUDE_DIRS
    LibVersioningCompiler_INCLUDES
    LibVersioningCompiler_LIBRARIES
    LibVersioningCompiler_LIBS
  )

cmake_minimum_required( VERSION 3.0.2 )

# target
set ( EXE_NAME "run_fft_00" )

# look for dependencies
list ( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake" )

find_package ( LibVersioningCompiler REQUIRED )
include_directories ( ${LIBVC_INCLUDES} )
link_directories( ${LIVC_LIB_DIR} )

# finalize dependencies
set ( DEP_LIBS ${DEP_LIBS} ${LIBVC_LIBRARIES} )

# sources
set ( SRC_WRAPPER ${CMAKE_CURRENT_SOURCE_DIR}/wrapper.cpp
                )

# compiler options
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -MD" )
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0" )

# finalize target
add_executable( ${EXE_NAME} ${SRC_WRAPPER} )
target_link_libraries( ${EXE_NAME} ${DEP_LIBS} )

# install path
if ( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
  set (CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
endif ()

install ( TARGETS ${EXE_NAME} DESTINATION bin )

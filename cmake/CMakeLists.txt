cmake_minimum_required(VERSION 3.2)

project(miso CXX)

set(MY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

macro(define_source_files)
  source_group(${ARGN} REGULAR_EXPRESSION ${ARGN})
  file(GLOB THIS_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../include/miso/${ARGN}/*.h")
  file(GLOB THIS_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../src/*${ARGN}*.cpp")
  list(APPEND SOURCE_FILES ${THIS_HEADERS} ${THIS_SOURCES})
  unset(THIS_HEADERS)
  unset(THIS_SOURCES)
endmacro()

define_source_files(socket)
define_source_files(client)
define_source_files(server)
define_source_files(protocol)

list(APPEND SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/miso/platform.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/miso/utility.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/platform.cpp")
source_group("" FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/miso/platform.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/miso/utility.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/platform.cpp")
list(APPEND SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/node_tcp.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/socket_tools.h")
source_group(internal FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/node_tcp.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/socket_tools.h")

include_directories("${MY_ROOT}/include")

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_range_for)

if(WIN32)
  target_link_libraries(${PROJECT_NAME} ws2_32.lib)
endif(WIN32)

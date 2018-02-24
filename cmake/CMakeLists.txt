cmake_minimum_required(VERSION 3.7)

project(miso CXX)

set(MY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

function (define_source_files GROUP_NAME GLOB_STRING VAR)
  file(GLOB THIS_SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${GLOB_STRING})
  source_group("${GROUP_NAME}" FILES ${THIS_SOURCE_FILES})
  list(APPEND ${VAR} ${THIS_SOURCE_FILES})
  set(${VAR} ${${VAR}} PARENT_SCOPE)
  unset(THIS_SOURCE_FILES)
endfunction ()

define_source_files("socket" "../include/miso/socket/*.h" SOURCE_FILES)
define_source_files("client" "../include/miso/client/*.h" SOURCE_FILES)
define_source_files("server" "../include/miso/server/*.h" SOURCE_FILES)
define_source_files("protocol" "../include/miso/protocol/*.h" SOURCE_FILES)
define_source_files("dns" "../include/miso/dns/*.h" SOURCE_FILES)

define_source_files("socket" "../src/*socket*.cpp" SOURCE_FILES)
define_source_files("client" "../src/*client*.cpp" SOURCE_FILES)
define_source_files("server" "../src/*server*.cpp" SOURCE_FILES)
define_source_files("protocol" "../src/*protocol*.cpp" SOURCE_FILES)
define_source_files("dns" "../src/*dns*.cpp" SOURCE_FILES)

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

if (WIN32)
  target_link_libraries(${PROJECT_NAME} ws2_32.lib)
endif ()

set(ARCHIVE_INSTALL_DIR "${MY_ROOT}/lib" CACHE STRING "Archive install dir")
set(LIBRARY_INSTALL_DIR "${MY_ROOT}/lib" CACHE STRING "Library install dir")
set(RUNTIME_INSTALL_DIR "${MY_ROOT}/bin" CACHE STRING "Runtime install dir")

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${ARCHIVE_INSTALL_DIR}
    LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
    RUNTIME DESTINATION ${RUNTIME_INSTALL_DIR})

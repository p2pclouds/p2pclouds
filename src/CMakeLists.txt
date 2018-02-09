# CMake minimum version requirements
cmake_minimum_required(VERSION 3.6)

# Name of the project
project (p2pclouds)


# Allow folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set (CMAKE_CXX_STANDARD 14)

# include directories
INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/libs
  ${PROJECT_SOURCE_DIR}/deps/utf8cpp
  ${PROJECT_SOURCE_DIR}/deps/kcp
  ${PROJECT_SOURCE_DIR}/deps/asio-1.10.8/include
  ${CMAKE_CURRENT_BINARY_DIR}/deps/gflags-2.2.0/include
  ${PROJECT_SOURCE_DIR}/deps/spdlog-0.16.3/include
)

# lib directories
LINK_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/deps/asio-1.10.8
  ${CMAKE_CURRENT_BINARY_DIR}/lib/
)

IF(UNIX)

INCLUDE_DIRECTORIES(
)

LINK_DIRECTORIES(
)

ELSE(UNIX)

INCLUDE_DIRECTORIES(
)

LINK_DIRECTORIES(
)

ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501) # asio required
ENDIF(UNIX)


# Precompiled macros
ADD_DEFINITIONS(-DASIO_STANDALONE -DASIO_SEPARATE_COMPILATION -DASIO_HAS_STD_CHRONO) # asio required
ADD_DEFINITIONS(-DLOG_TO_CONSOLE) # log required

# add subdirectory
add_subdirectory(deps/gflags-2.2.0)
add_subdirectory(deps/kcp)
add_subdirectory(deps/asio-1.10.8)
add_subdirectory(deps/openssl)

add_subdirectory(libs/app)
add_subdirectory(libs/network)
add_subdirectory(libs/common)
add_subdirectory(libs/log)
add_subdirectory(libs/kademlia)
add_subdirectory(libs/blockchain)

add_subdirectory(apps/p2pclouds)
add_subdirectory(apps/test)

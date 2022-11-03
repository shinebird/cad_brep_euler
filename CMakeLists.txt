# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.13)

project ("CAD-Homework1" LANGUAGES CXX)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    message(WARNING "This project only tested Windows for now")
endif()
if(NOT MSVC)
    message(WARNING "This project only tested on native Windows MSVC-like compilers for now")
endif()
if(NOT (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "AMD64"))
    message(FATAL_ERROR "This project only supports x86_64 CPU")
endif()
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# 包含子项目。
add_subdirectory ("src")

cmake_minimum_required(VERSION 3.16)
project(RenderEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default compile flags
#if(CMAKE_COMPILER_IS_GNUCXX)
#    message(STATUS "GCC detected, adding compile flags")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
#endif(CMAKE_COMPILER_IS_GNUCXX)
#
#if(MSVC)
#    message(STATUS "MSVC detected, adding compile flags")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4 -WX")
#endif(MSVC)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(EnableAddressSanitizer)
include(EnableUndefinedSanitizer)
include(clang-tidy)
include(BundledGTest)

add_subdirectory(src)
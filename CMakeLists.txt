cmake_minimum_required(VERSION 3.16)
project(ChaosEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default compile flags
if (CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
elseif (MSVC)
    message(STATUS "MSVC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
else ()
    message(FATAL_ERROR "Unsupported OS")
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(EnableAddressSanitizer)
include(EnableUndefinedSanitizer)
include(clang-tidy)
include(BundledGTest)

add_subdirectory(dep/nativefiledialog-extended)
#set(CMAKE_CXX_CLANG_TIDY
#        clang-tidy;
#        -header-filter=.*
#        -checks=*;
#        )

add_subdirectory(Engine)
add_subdirectory(Sandbox)

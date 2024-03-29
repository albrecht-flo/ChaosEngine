set(DISABLE_UBSAN OFF CACHE BOOL "disable compilation with the sanitizer for undefined behavior for Debug builds")

include(CheckCXXCompilerFlag)

if (NOT DISABLE_UBSAN AND CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_REQUIRED_FLAGS "-Werror -fsanitize=undefined")
    check_cxx_compiler_flag("-fsanitize=undefined" HAVE_FLAG_SANITIZE_UNDEFINED)
    unset(CMAKE_REQUIRED_FLAGS)

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")

    if (NOT HAVE_FLAG_SANITIZE_UNDEFINED)
        message(WARNING "The undefined behavior sanitizer is enabled but not supported on your system. Please disable (-DDISABLE_UBSAN=ON) the undefined behavior sanitizer or choose a compiler that supports it.")
    else()
        message(STATUS "Undefined behavior sanitizer enabled")
    endif ()
elseif(NOT DISABLE_UBSAN)
    message(STATUS "Undefined behavior sanitizer not supported by compiler")
else ()
    message(STATUS "Undefined behavior sanitizer disabled by parameter DISABLE_UBSAN")
endif ()
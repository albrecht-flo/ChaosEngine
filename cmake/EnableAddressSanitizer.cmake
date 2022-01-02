set(DISABLE_ASAN OFF CACHE BOOL "disable compilation with the address sanitizer for Debug builds")

include(CheckCXXCompilerFlag)

if (NOT DISABLE_ASAN AND CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_REQUIRED_FLAGS "-Werror -fsanitize=address")
    check_cxx_compiler_flag(-fsanitize=address HAVE_FLAG_SANITIZE_ADDRESS)
    unset(CMAKE_REQUIRED_FLAGS)

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

    if (NOT HAVE_FLAG_SANITIZE_ADDRESS)
        message(WARNING "The address sanitizer is enabled but not supported on your system. Either disable the address sanitizer (-DDISABLE_ASAN=ON), choose a compiler that supports it, or fix your system (most likely by installing libasan).")
    else ()
        message(STATUS "Address sanitizer enabled")
    endif ()
elseif(NOT DISABLE_ASAN)
    message(STATUS "Address sanitizer not supported by compiler")
else ()
    message(STATUS "Address sanitizer disabled by parameter DISABLE_ASAN")
endif ()
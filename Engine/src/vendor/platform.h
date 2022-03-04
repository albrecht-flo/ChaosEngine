#pragma once

#ifdef _MSC_VER
#define PUSH_MINIMAL_WARNING_LEVEL __pragma(warning(push, 0))
#define POP_MINIMAL_WARNING_LEVEL __pragma(warning(pop))
#else
// No warnings for third party libraries
#define PUSH_MINIMAL_WARNING_LEVEL \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wunused-variable\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-but-set-variable\"") \
    _Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"") \
    _Pragma("GCC diagnostic ignored \"-Wtype-limits\"") \
    _Pragma("GCC diagnostic ignored \"-Wsign-compare\"") \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-enum-enum-conversion\"")

#define POP_MINIMAL_WARNING_LEVEL _Pragma("GCC diagnostic pop")
#endif

// MSVC does not support __PRETTY_FUNCTION__ but calls it __FUNCSIG__
#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

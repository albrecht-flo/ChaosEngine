#include "STDExtensions.h"

#include <algorithm>

std::string ChaosEngine::stringToLower(const std::string &str) {
    std::string copy(str);
    std::transform(copy.begin(), copy.end(), copy.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return copy;
}
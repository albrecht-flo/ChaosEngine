#pragma once

#include <string>
#include <vector>

namespace ChaosEngine::AssetLoader {
    std::string loadString(const std::string& filePath);

    std::vector<char> loadBinary(const std::string &filePath);
}




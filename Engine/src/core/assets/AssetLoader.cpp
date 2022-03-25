#include "AssetLoader.h"

#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace ChaosEngine::AssetLoader {

    std::string loadString(const std::string &filePath) {
        if (!exists(fs::path(filePath)))
            throw std::runtime_error("Requested file does not exist: '" + filePath + "'");

        std::ifstream input(filePath);
        if (!input.is_open()) {
            throw std::runtime_error("Requested file could not be opened: '" + filePath + "'");
        }

        std::stringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
    }

    std::vector<char> loadBinary(const std::string &filePath) {
        if (!exists(fs::path(filePath)))
            throw std::runtime_error("Requested file does not exist: '" + filePath + "'");

        std::ifstream input(filePath, std::ios::binary);
        if (!input.is_open()) {
            throw std::runtime_error("Requested file could not be opened: '" + filePath + "'");
        }

        // Get the length of the file
        input.seekg(0, std::ios::end);
        auto length = input.tellg();
        input.seekg(0, std::ios::beg);

        std::vector<char> buffer(length);
        input.read(buffer.data(), length);
        return buffer;
    }
}

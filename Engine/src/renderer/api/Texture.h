#pragma once

#include <string>
#include <memory>

namespace Renderer {
    class Texture {
    public:
        virtual ~Texture() = default;

        static std::unique_ptr<Texture> Create(const std::string& filename);
    };
}




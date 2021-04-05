#pragma once

#include <string>

#include "Engine/src/renderer/api/Buffer.h"

#include <memory>

namespace Renderer {

    class RenderMesh {
    public:
        explicit RenderMesh(uint32_t indexCount) : indexCount(indexCount), indexed(false) {}

        virtual ~RenderMesh() = default;

        static std::unique_ptr<RenderMesh>
        Create(std::unique_ptr<Buffer> &&vertexBuffer, std::unique_ptr<Buffer> &&indexBuffer, size_t indexCount);

        inline uint32_t getIndexCount() const { return indexCount; }

        inline bool isIndexed() const { return indexed; }

    private:
        uint32_t indexCount;
        bool indexed;
    };

}



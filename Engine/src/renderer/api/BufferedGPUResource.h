#pragma once

#include <memory>

class BufferedGPUResource {
public:
    virtual ~BufferedGPUResource() = default;

    virtual void destroy() = 0;
};

struct BufferedGPUResourceEntry {
    std::unique_ptr<BufferedGPUResource> resource;
    uint32_t frameDeleted;
};



#pragma once

#include <memory>
#include <string>

class BufferedGPUResource {
public:
    virtual ~BufferedGPUResource() = default;

    virtual void destroy() = 0;

    virtual std::string toString() const = 0;
};

struct BufferedGPUResourceEntry {
    BufferedGPUResourceEntry(std::unique_ptr<BufferedGPUResource> &&resource, uint32_t frameDeleted)
            : resource(std::move(resource)), frameDeleted(frameDeleted) {}

    std::unique_ptr<BufferedGPUResource> resource;
    uint32_t frameDeleted;
};



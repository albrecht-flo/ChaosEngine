#pragma once

#include <src/renderer/vulkan/pipeline/VulkanVertexInput.h>
#include "pipeline/VulkanPipeline.h"

class VulkanDataManager {
public:
    VulkanDataManager() = default;

    ~VulkanDataManager() = default;

    VulkanDataManager(const VulkanDataManager &o) = delete;

    VulkanDataManager &operator=(const VulkanDataManager &o) = delete;

    VulkanDataManager(VulkanDataManager &&o) noexcept;

    VulkanDataManager &operator=(VulkanDataManager &&o) = delete;

    void addNewPipeline(VulkanPipeline &&input);


private:
    std::vector<VulkanPipeline> pipelines;
};


#include "VulkanDataManager.h"

// ------------------------------------ Class Members ------------------------------------------------------------------

VulkanDataManager::VulkanDataManager(VulkanDataManager &&o) noexcept
        : pipelines(std::move(o.pipelines)) {}

void VulkanDataManager::addNewPipeline(VulkanPipeline &&input) {
    pipelines.emplace_back(std::move(input));
}

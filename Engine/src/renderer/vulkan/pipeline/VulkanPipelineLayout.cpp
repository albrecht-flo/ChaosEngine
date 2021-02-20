#include "VulkanPipelineLayout.h"

VulkanPipelineLayout &VulkanPipelineLayout::operator=(VulkanPipelineLayout &&o) noexcept {
    if (this == &o)
        return *this;
    destroy();
    layout = std::exchange(o.layout, nullptr);
    return *this;
}

void VulkanPipelineLayout::destroy() {
    if (layout != nullptr) vkDestroyPipelineLayout(device.vk(), layout, nullptr);
}
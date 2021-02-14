#include "VulkanDescriptorSetLayout.h"

void VulkanDescriptorSetLayout::destroy() {
    if (layout != nullptr) vkDestroyDescriptorSetLayout(device.vk(), layout, nullptr);
}



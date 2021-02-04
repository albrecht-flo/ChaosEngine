#include "VulkanVertexInput.h"

#include <utility>

#include <glm/glm.hpp>

VulkanVertexInput::VulkanVertexInput(std::vector<VkVertexInputBindingDescription> &&bindingDescription,
                                     std::vector<VkVertexInputAttributeDescription> &&attributeDescriptions)
        : attributeDescriptions(std::move(attributeDescriptions)), bindingDescription(std::move(bindingDescription)) {}

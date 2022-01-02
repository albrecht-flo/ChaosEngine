#include "VulkanVertexInput.h"

#include <utility>

#include <glm/glm.hpp>

VulkanVertexInput::VulkanVertexInput(std::vector<VkVertexInputBindingDescription> &&bindingDescription,
                                     std::vector<VkVertexInputAttributeDescription> &&attributeDescriptions)
        : bindingDescription(std::move(bindingDescription)), attributeDescriptions(std::move(attributeDescriptions)) {}

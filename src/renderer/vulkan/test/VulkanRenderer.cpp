#include "VulkanRenderer.h"

#include <stdexcept>

// TODO: Remove this might throw an exception during construction
VulkanRenderer::VulkanRenderer(Window &w) :
        window(w),
        instance(VulkanInstance::Create({"VK_LAYER_KHRONOS_validation"}, "Old", "Old")),
        surface(window.createSurface(instance.getInstance())),
        device(VulkanDevice::Create(instance, surface)) {
    // Let the child class handle the rest of initialization in init
}

VulkanRenderer::~VulkanRenderer() = default;

/* Creates the Vulkan surface from the window. */
void VulkanRenderer::createSurface() {
    if (glfwCreateWindowSurface(instance.getInstance(), window.getWindow(), nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create window surface!");
    }
}

/* Waits for all actions on the device to finish. */
void VulkanRenderer::waitIdle() {
    device.waitIdle();
}

void VulkanRenderer::cleanup() {
    // Let the child class cleanup its things
    destroyResources();


    vkDestroySurfaceKHR(instance.getInstance(), surface, nullptr);
}

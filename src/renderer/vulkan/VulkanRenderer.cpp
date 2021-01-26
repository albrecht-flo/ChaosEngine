#include "VulkanRenderer.h"

#include <stdexcept>

VulkanRenderer::VulkanRenderer(Window &w) :
        window(w) {

    // Vulkan context
    instance = VulkanInstance::create({"VK_LAYER_KHRONOS_validation"});
    createSurface();
    device.init(instance, surface);

    // Let the child class handle the rest of initialization in init
}

VulkanRenderer::~VulkanRenderer() {}

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

    // Destroy the device and instance
    device.destroy();

    vkDestroySurfaceKHR(instance.getInstance(), surface, nullptr);

    VulkanInstance::destroy(instance);
}

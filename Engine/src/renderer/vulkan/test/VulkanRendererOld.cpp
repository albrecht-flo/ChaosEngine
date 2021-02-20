#include "VulkanRendererOld.h"

#include <stdexcept>

// TODO: Remove this might throw an exception during construction
VulkanRendererOld::VulkanRendererOld(Window &w) :
        window(w),
        instance(VulkanInstance::Create({"VK_LAYER_KHRONOS_validation"}, "Old", "Old")),
        surface(window.createSurface(instance.vk())),
        device(VulkanDevice::Create(instance, surface)) {
    // Let the child class handle the rest of initialization in init
}

VulkanRendererOld::~VulkanRendererOld() = default;

/* Creates the Vulkan surface from the window. */
void VulkanRendererOld::createSurface() {
    if (glfwCreateWindowSurface(instance.vk(), window.getWindow(), nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create window surface!");
    }
}

/* Waits for all actions on the device to finish. */
void VulkanRendererOld::waitIdle() {
    device.waitIdle();
}

void VulkanRendererOld::cleanup() {
    // Let the child class cleanup its things
    destroyResources();


    vkDestroySurfaceKHR(instance.vk(), surface, nullptr);
}

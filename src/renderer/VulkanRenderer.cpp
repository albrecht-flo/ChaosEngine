#include "VulkanRenderer.h"

#include <stdexcept>

VulkanRenderer::VulkanRenderer(Window &w) :
        m_window(w) {

    // Vulkan context
    m_instance = VulkanInstance::create({"VK_LAYER_KHRONOS_validation"});
    createSurface();
    m_device.init(m_instance, m_surface);

    // Let the child class handle the rest of initialization in init
}

VulkanRenderer::~VulkanRenderer() {}

/* Creates the Vulkan surface from the window. */
void VulkanRenderer::createSurface() {
    if (glfwCreateWindowSurface(m_instance.getInstance(), m_window.getWindow(), nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create window surface!");
    }
}

/* Waits for all actions on the device to finish. */
void VulkanRenderer::waitIdle() {
    m_device.waitIdle();
}

void VulkanRenderer::cleanup() {
    // Let the child class cleanup its things
    destroyResources();

    // Destroy the device and instance
    m_device.destroy();

    vkDestroySurfaceKHR(m_instance.getInstance(), m_surface, nullptr);

    VulkanInstance::destroy(m_instance);
}

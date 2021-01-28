#include "VulkanContext.h"

#include <stdexcept>

VulkanContext::VulkanContext(Window &window) :
        window(window),
        device(),
        instance(),
        surface(),
        swapChain{device, surface, window}
        {}


void VulkanContext::init() {
    instance.init({"VK_LAYER_KHRONOS_validation"});
    window.createSurface(instance.getInstance(), &surface);
    device.init(instance, surface);
    swapChain.init();
}
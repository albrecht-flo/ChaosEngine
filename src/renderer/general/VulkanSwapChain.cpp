#include "VulkanSwapChain.h"

#include "../image/VulkanImageView.h"

#include <array>
#include <stdexcept>

VulkanSwapChain::VulkanSwapChain() {}

VulkanSwapChain::~VulkanSwapChain() {}

/* Initialized the swap chain and creates the image views */
void VulkanSwapChain::init(VulkanDevice *device, Window *window, VkSurfaceKHR *surface) {
    m_device = device;
    m_window = window;
    m_surface = surface;
    createSwapChain();
    createImageViews();
}

/* Destories frame buffers, image views and swap chain. */
void VulkanSwapChain::destroy() {
    for (auto imageView : m_swapChainImageViews) {
        VulkanImageView::destroy(*m_device, imageView);
    }

    vkDestroySwapchainKHR(m_device->getDevice(), m_swapChain, nullptr);
}

/* Reinitializes the swap chain. */
void VulkanSwapChain::reinit() {
    createSwapChain();
    createImageViews();
}

/* Helper to select an apropriate surface format. */
VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

/* Helper to select an apropriate present mode. */
VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

/* Helper to select the optimal and supported dimensions/extent based on the window. */
VkExtent2D VulkanSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        // Retrieve current dimensions
        int width, height;
        glfwGetFramebufferSize(m_window->getWindow(), &width, &height);

        VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
        };

        // Clamp dimensions to supported and keep within bounds
        actualExtent.width = std::max(capabilities.minImageExtent.width,
                                      std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
                                       std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

/* Creates the swapchain and its images. */
void VulkanSwapChain::createSwapChain() {
    // Check for available swapchain
    SwapChainSupportDetails swapChainSupport = m_device->querySwapChainSupport();

    // Select our surface and dimensions
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // Use at least one more image than the supported minimum, to avoid having to wait for the driver to finish with the first one
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    // Limit to max images
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Create the swapchain
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = *m_surface; // the surface to use
    // Specify the images and their format
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // images are used for color output // ~ TRANSFER for post processing

    // Determine how the images will be accessed by different queues
    QueueFamilyIndices indices = m_device->findQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) { // two different queues for rendering and presentation
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // no explicit ownership transfer needs to be done
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // ownership needs to be handled explicitly
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // we do not want any transform to the final image
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // no blending with other windows
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // discard pixels covered by other windows

    if (vkCreateSwapchainKHR(m_device->getDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create swap chain!");
    }

    // Retrieve images
    vkGetSwapchainImagesKHR(m_device->getDevice(), m_swapChain, &imageCount, nullptr); // get image count
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device->getDevice(), m_swapChain, &imageCount,
                            m_swapChainImages.data()); // store in vector

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

/* Creates image views for the images of the swapchain. */
void VulkanSwapChain::createImageViews() {
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        m_swapChainImageViews[i] = VulkanImageView::create(*m_device, m_swapChainImages[i], m_swapChainImageFormat,
                                                           VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

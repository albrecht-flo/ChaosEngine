#include "VulkanSwapChain.h"

#include "Engine/src/renderer/window/Window.h"
#include "VulkanDevice.h"
#include "Engine/src/renderer/vulkan/image/VulkanImageView.h"
#include "Engine/src/renderer/vulkan/image/VulkanFramebuffer.h"
#include "Engine/src/renderer/vulkan/rendering/VulkanRenderPass.h"

#include <array>
#include <stdexcept>
#include <tuple>
#include <iostream>
#include <cassert>

/* Helper to select an appropriate surface format. */
static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    auto it = std::find_if(availableFormats.begin(), availableFormats.end(),
                           [](auto format) {
                               return format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                                      format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                           });

    if (it != availableFormats.end())
        return *it;

    return availableFormats[0];
}

/* Helper to select an appropriate present mode. */
static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    auto it = std::find_if(availablePresentModes.begin(), availablePresentModes.end(),
                           [](auto mode) { return mode == VK_PRESENT_MODE_FIFO_KHR; });

    if (it != availablePresentModes.end())
        return *it;

    return VK_PRESENT_MODE_FIFO_KHR;
}

/* Helper to select the optimal and supported dimensions/extent based on the window. */
static VkExtent2D chooseSwapExtent(const Window &window, const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        // Retrieve current dimensions
        const auto fbSize = window.getFrameBufferSize();

        VkExtent2D actualExtent = {
                static_cast<uint32_t>(fbSize.x),
                static_cast<uint32_t>(fbSize.y)
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
static std::tuple<VkSwapchainKHR, VkFormat, VkExtent2D>
createSwapChain(const Window &window, const VulkanDevice &device, VkSurfaceKHR surface) {
    // Check for available swapchain
    SwapChainSupportDetails swapChainSupport = device.querySwapChainSupport(surface);
    assert("Expect surface KHR to be supported" && device.checkSurfaceAvailability(surface));

    // Select our surface and dimensions
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(window, swapChainSupport.capabilities);

    // Use at least one more image than the supported minimum, to avoid having to wait for the driver to finish with the first one
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    // Limit to max images
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Create the swapchain
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface; // the surface to use
    // Specify the images and their format
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // images are used for color output // ~ TRANSFER for post processing
    createInfo.oldSwapchain = VK_NULL_HANDLE; // To speed up swapchain recreation

    // Determine how the images will be accessed by different queues
    QueueFamilyIndices indices = device.findQueueFamilies(surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) { // two different queues for rendering and presentation
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // no explicit ownership transfer needs to be done
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // ownership needs to be handled explicitly
    }

    createInfo.preTransform =
            swapChainSupport.capabilities.currentTransform; // we do not want any transform to the final image
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // no blending with other windows
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // discard pixels covered by other windows

    VkSwapchainKHR swapChain{};
    if (vkCreateSwapchainKHR(device.vk(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to create swap chain!");
    }

    return std::make_tuple(swapChain, surfaceFormat.format, extent);
}

/* Creates image views for the images of the swapchain. */
static std::vector<VulkanImageView>
createImageViews(const VulkanDevice &device, const std::vector<VkImage> &swapChainImages,
                 VkFormat swapChainImageFormat) {
    std::vector<VulkanImageView> swapChainImageViews;
    swapChainImageViews.reserve(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews.emplace_back(VulkanImageView::Create(device, swapChainImages[i], swapChainImageFormat,
                                                                 VK_IMAGE_ASPECT_COLOR_BIT));
    }

    return swapChainImageViews;
}

// Retrieves the images if the swapchain
static std::vector<VkImage>
getSwapChainImages(VkDevice device, VkSwapchainKHR swapChain) {
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr); // get image count

    std::vector<VkImage> swapChainImages;
    swapChainImages.resize(imageCount);

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()); // store in vector

    return swapChainImages;
}

// ------------------------------------ Class Methods ------------------------------------------------------------------


/* Initialized the swap chain and creates the image views */
VulkanSwapChain VulkanSwapChain::Create(const Window &window, const VulkanDevice &device, VkSurfaceKHR surface) {
    auto[swapChain, swapChainImageFormat, swapChainExtent] = createSwapChain(window, device, surface);

    std::vector<VkImage> swapChainImages = getSwapChainImages(device.vk(), swapChain);

    auto swapChainImageViews = createImageViews(device, swapChainImages, swapChainImageFormat);

    return VulkanSwapChain{window, device, surface,
                           swapChain, swapChainImageFormat, swapChainExtent,
                           std::move(swapChainImages), std::move(swapChainImageViews)
    };
}

VulkanSwapChain::VulkanSwapChain(const Window &window, const VulkanDevice &device, VkSurfaceKHR surface,
                                 VkSwapchainKHR swapChain, VkFormat swapChainImageFormat, VkExtent2D swapChainExtent,
                                 std::vector<VkImage> &&swapChainImages,
                                 std::vector<VulkanImageView> &&swapChainImageViews)
        : window(window), device(device), surface(surface),
          swapChain(swapChain), swapChainImageFormat(swapChainImageFormat), swapChainExtent(swapChainExtent),
          swapChainImages(std::move(swapChainImages)), swapChainImageViews(std::move(swapChainImageViews)) {}

VulkanSwapChain::VulkanSwapChain(VulkanSwapChain &&o) noexcept
        : window(o.window), device(o.device), surface(o.surface),
          swapChain(std::exchange(o.swapChain, nullptr)), swapChainImageFormat(o.swapChainImageFormat),
          swapChainExtent(o.swapChainExtent),
          swapChainImages(std::move(o.swapChainImages)), swapChainImageViews(std::move(o.swapChainImageViews)) {}

VulkanSwapChain::~VulkanSwapChain() {
    destroy();
}

/* Stories frame buffers, image views and swap chain. */
void VulkanSwapChain::destroy() {
    swapChainImageViews.clear();
    if (swapChain != nullptr) {
        vkDestroySwapchainKHR(device.vk(), swapChain, nullptr);
        swapChain = nullptr;
    }
}

void VulkanSwapChain::recreate(VkSurfaceKHR mSurface) {
    surface = mSurface;
    auto[mSwapChain, mSwapChainImageFormat, mSwapChainExtent] = createSwapChain(window, device, surface);
    swapChain = mSwapChain;
    swapChainImageFormat = mSwapChainImageFormat;
    swapChainExtent = mSwapChainExtent;

    swapChainImages = getSwapChainImages(device.vk(), swapChain);

    swapChainImageViews = createImageViews(device, swapChainImages, swapChainImageFormat);
}

std::vector<VulkanFramebuffer> VulkanSwapChain::createFramebuffers(const VulkanRenderPass &renderPass) const {
    using namespace Renderer;
    std::vector<VulkanFramebuffer> swapChainFramebuffers;
    swapChainFramebuffers.reserve(size());
    for (uint32_t i = 0; i < size(); i++) {
        swapChainFramebuffers.emplace_back(
                renderPass.createFrameBuffer(
                        {FramebufferAttachmentInfo{AttachmentType::SwapChain, AttachmentFormat::SwapChain, i}},
                        getWidth(), getHeight(), "Swapchain" + std::to_string(i))
        );
    }
    return swapChainFramebuffers;
}

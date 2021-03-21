#include "VulkanFrame.h"

#include "Engine/src/renderer/vulkan/context/VulkanContext.h"

#include <array>
#include <vector>
#include <stdexcept>

static std::vector<VkFence> createFence(const VulkanContext &context, uint32_t amount) {
    std::vector<VkFence> fences;
    fences.resize(amount);

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < amount; i++) {
        if (vkCreateFence(context.getDevice().vk(), &fenceInfo, nullptr, &fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("[VULKAN] Failed to create fence!");
        }
        char t[] = "Frame Fence 0";
        t[12] = static_cast<char>('0' + (char) i);

        context.setDebugName(VK_OBJECT_TYPE_FENCE, (uint64_t) fences[i], t);
    }

    return std::move(fences);
}

static std::vector<VkSemaphore> createSemaphore(VkDevice device, uint32_t amount) {
    std::vector<VkSemaphore> semaphores;
    semaphores.resize(amount);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < amount; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("[VULKAN] Failed to create semaphore!");
        }
    }

    return std::move(semaphores);
}

// ------------------------------------ Class members ------------------------------------------------------------------

VulkanFrame VulkanFrame::Create(Window &window, const VulkanContext &context, uint32_t maxFramesInFlight) {
    auto imageAvailableSemaphores = createSemaphore(context.getDevice().vk(), maxFramesInFlight);
    auto renderFinishedSemaphores = createSemaphore(context.getDevice().vk(), maxFramesInFlight);
    auto inFlightFences = createFence(context, maxFramesInFlight);

    return VulkanFrame(window, context, std::move(imageAvailableSemaphores), std::move(renderFinishedSemaphores),
                       std::move(inFlightFences));
}

VulkanFrame::VulkanFrame(VulkanFrame &&o) noexcept
        : window(o.window), context(o.context), imageAvailableSemaphores(std::move(o.imageAvailableSemaphores)),
          renderFinishedSemaphores(std::move(o.renderFinishedSemaphores)),
          inFlightFences(std::move(o.inFlightFences)) {}

VulkanFrame::VulkanFrame(Window &window, const VulkanContext &context,
                         std::vector<VkSemaphore> &&imageAvailableSemaphores,
                         std::vector<VkSemaphore> &&renderFinishedSemaphores, std::vector<VkFence> &&inFlightFences)
        : window(window), context(context), imageAvailableSemaphores(std::move(imageAvailableSemaphores)),
          renderFinishedSemaphores(std::move(renderFinishedSemaphores)),
          inFlightFences(std::move(inFlightFences)) {}

VulkanFrame::~VulkanFrame() {
    destroy();
};

void VulkanFrame::destroy() {
    // Destroy the synchronization objects
    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) {
        vkDestroySemaphore(context.getDevice().vk(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(context.getDevice().vk(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(context.getDevice().vk(), inFlightFences[i], nullptr);
    }
}


bool VulkanFrame::render(size_t currentFrame, const VulkanCommandBuffer &commandBuffer) const {
    // Wait for the old frame to finish rendering
    vkWaitForFences(context.getDevice().vk(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // Acquire the next image to draw to from the swapchain
    // Specifies the sync objects to be notified when the image is ready
    uint32_t imageIndex; // index of available image
    VkResult result = vkAcquireNextImageKHR(context.getDevice().vk(),
                                            context.getSwapChain().getSwapChain(), UINT64_MAX /*timeout*/,
                                            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) { // swap chain has been invalidated
        return false;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("[Vulkan] Failed to acquire swap chain image!");
    }

    // Prepare to submit the command buffers for the current frame
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]}; // wait for the image to be available
    VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // wait for the image to be writeable before writing the color output
    // This means that the vertex shader stage etc. can already be executed
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores; // semaphore to wait for before executing
    submitInfo.pWaitDstStageMask = waitStages; // stages to wait for before executing
    // Specify the command buffers to submit for this draw call
    std::array<VkCommandBuffer, 1> activeCommandBuffers = {commandBuffer.vk()};
    submitInfo.commandBufferCount = static_cast<uint32_t>(activeCommandBuffers.size());
    submitInfo.pCommandBuffers = activeCommandBuffers.data();
    // Specify semaphore to notify after finishing
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Reset the finish fence
    vkResetFences(context.getDevice().vk(), 1, &inFlightFences[currentFrame]);

    // Submit the command buffers to the queue and the fence to be notified after finishing this frame
    VkResult res;
    if ((res = vkQueueSubmit(context.getDevice().getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame])) !=
        VK_SUCCESS) {
        std::cerr << "vkQueueSubmit Failed with result = " << res << std::endl;
        throw std::runtime_error("[Vulkan] Failed to submit draw command buffer!");
    }

    // Present the frame on the screen
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // Wait for the frame to be finished
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    // Specify the swapchain for presentation
    VkSwapchainKHR swapChains[] = {context.getSwapChain().getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    // Specify the image to present
    presentInfo.pImageIndices = &imageIndex;
    // Present the frame
    result = vkQueuePresentKHR(context.getDevice().getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        window.getFrameBufferResize()) { // swapchain is invalid
        window.setFrameBufferResized(false);
        return false;
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to present swap chain image!");
    }
    return true;
}

void VulkanFrame::waitUntilCurrentFrameIsFree(uint32_t currentFrame) const {
    // Wait until the frame that we are going to record in the next cycle has finished rendering
    vkWaitForFences(context.getDevice().vk(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
}

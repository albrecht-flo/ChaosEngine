#include "TestRenderer.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

#include "src/renderer/data/ModelLoader.h"

TestRenderer::TestRenderer(Window &w) :
        VulkanRenderer(w),
        swapChain(VulkanSwapChain::Create(window, device, surface)),
        vulkanMemory(device),
        mainGraphicsPass(device, vulkanMemory, swapChain),
        imGuiRenderPass(device, vulkanMemory, swapChain, window, instance),
        postRenderPass(device, vulkanMemory, swapChain) {}

void TestRenderer::init() {
    // Vulkan Instance and Device are handled by VulkanRenderer constructor

    // GPU communication
    createCommandPool();
    // Create per frame command buffers
    createCommandBuffers();
    // Sync objects for drawing frames
    createSyncObjects();

    // Initialize memory manager
    vulkanMemory.init(commandPool);

    // requires memory manager for depth image creation
    createDepthResources();
    createImageBuffers();

    // Render pass
    mainGraphicsPass.init();
    imGuiRenderPass.init();
    postRenderPass.init();
    postRenderPass.setImageBufferViews(offscreenImageView, depthImageView, imGuiImageView);

    createFramebuffers();

    auto quad = ModelLoader::getQuad();
    quadMesh = uploadMesh(quad);
    quadRobj.mesh = &quadMesh;
    quadRobj.modelMat = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, -3.0f));
}


/* Creates command pool to contain command buffers */
void TestRenderer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = device.findQueueFamilies();

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // We want the buffers to be able to reset them
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device.getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create graphics command pool!");
    }
}

/* Creates the command buffers for all frames from the command pool. */
void TestRenderer::createCommandBuffers() {
    commandBuffers.clear();
    commandBuffers.reserve(swapChain.size());

    for (size_t i = 0; i < swapChain.size(); i++) {
        commandBuffers.emplace_back(VulkanCommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY));
    }
}

/* Creates the objects for synchronization between frames. */
void TestRenderer::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) !=
            VK_SUCCESS ||
            vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) !=
            VK_SUCCESS ||
            vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("VULKAN: failed to create synchronization objects for a frame!");
        }
    }
}

/* Creates the depth resources for depth buffering. */
void TestRenderer::createDepthResources() {
    VkFormat depthFormat = VulkanImage::getDepthFormat(device);

    depthImage = VulkanImage::createDepthBufferImage(
            device, vulkanMemory, swapChain.getExtent().width,
            swapChain.getExtent().height,
            depthFormat, depthImageMemory);
    depthImageView = VulkanImageView::create(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

/* Creates images and image views for offscreen buffers. */
void TestRenderer::createImageBuffers() {
    // Offscreen main scene rendering
    offscreenImage =
            VulkanImage::createRawImage(device,
                                        vulkanMemory, swapChain.getExtent().width, swapChain.getExtent().height,
                                        VK_FORMAT_R8G8B8A8_UNORM,
                                        offscreenImageMemory);
    offscreenImageView =
            VulkanImageView::create(device,
                                    offscreenImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

    // Offscreen ImGui rendering
    imGuiImage =
            VulkanImage::createRawImage(device,
                                        vulkanMemory, swapChain.getExtent().width, swapChain.getExtent().height,
                                        VK_FORMAT_R8G8B8A8_UNORM,
                                        imGuiImageMemory);
    imGuiImageView =
            VulkanImageView::create(device,
                                    imGuiImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

/* Creates framebuffers for offscreen render passes and final composite pass. */
void TestRenderer::createFramebuffers() {
    // Create offscreen frame buffer // ONLY ONE because we only have one frame in active rendering
    offscreenFramebuffer = VulkanFramebuffer::createFramebuffer(
            device,
            {offscreenImageView, depthImageView},
            mainGraphicsPass.getVkRenderPass(),
            swapChain.getExtent().width, swapChain.getExtent().height
    );
    // Create offscreen ImGui framebuffer
    imGuiFramebuffer = VulkanFramebuffer::createFramebuffer(
            device,
            {imGuiImageView},
            imGuiRenderPass.getVkRenderPass(),
            swapChain.getExtent().width, swapChain.getExtent().height
    );

    // Create framebuffers for swap chain
    swapChainFramebuffers.resize(swapChain.size());
    for (uint32_t i = 0; i < swapChain.size(); i++) {
        swapChainFramebuffers[i] = VulkanFramebuffer::createFramebuffer(
                device,
                {swapChain.getImageViews()[i]},
                postRenderPass.getVkRenderPass(),
                swapChain.getExtent().width, swapChain.getExtent().height
        );
    }
}


/* Updates the uniform buffer for the current image. (per frame) */
void TestRenderer::updateUniformBuffer(uint32_t currentImage) {
    // Create the new uniform buffer data

    LightObject worldLight = {
            .lightPos = glm::vec4(+100.0f, 100.0f, 0.0f, 500.0f),
            .lightColor = glm::vec4(1.0f, 1.0f, 0.9f, 0.5f)
    };
    mainGraphicsPass.updateUniformBuffer(currentImage, camera, worldLight);
    postRenderPass.updateCamera(camera);
}

void TestRenderer::updateCommandBuffer(uint32_t currentImage) {
    // Implicitly resets the command buffer.
    commandBuffers[currentImage].begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    VkCommandBuffer cmdBuf = commandBuffers[currentImage].getBuffer();

    // Let the render pass setup context descriptors
    mainGraphicsPass.cmdBegin(cmdBuf, currentImage, offscreenFramebuffer);

    for (auto &robj : renderObjects) {
        // Let the render pass setup per model descriptors
        mainGraphicsPass.cmdRender(cmdBuf, robj);

        // Bind the vertex buffer
        VkBuffer vertexBuffers[]{robj.mesh->vertexBuffer.buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmdBuf, 0, 1, vertexBuffers, offsets);
        // Bind the index buffer
        vkCmdBindIndexBuffer(cmdBuf, robj.mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmdBuf, robj.mesh->indexCount, 1, 0, 0, 0);
    }

    // End this render pass
    mainGraphicsPass.cmdEnd(cmdBuf);

    // !!! Transition of image layouts performed by attachment final layouts
    // !!! synchronization done by mainGraphicsPass->dependency.dstSubPass EXTERNAL
    // !!!							imGuiRenderPass->dependency.srcSubPass EXTERNAL

    imGuiRenderPass.cmdBegin(cmdBuf, currentImage, imGuiFramebuffer);
    // No more needed, ImGui rendering is called in cmdBegin
    imGuiRenderPass.cmdEnd(cmdBuf);

    // !!! Transition of image layouts performed by attachment final layouts
    // !!! synchronization done by imGuiRenderPass->dependency.dstSubPass EXTERNAL
    // !!!							postRenderPass->dependency.srcSubPass EXTERNAL

    // Start post processing render pass
    postRenderPass.cmdBegin(cmdBuf, currentImage, swapChainFramebuffers[currentImage]);
    // Bind previous color attachment to fragment shader sampler
    postRenderPass.cmdRender(cmdBuf, quadRobj);
    // Bind a screen filling quad to render the previous render passes to
    VkBuffer vertexBuffers[]{quadRobj.mesh->vertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmdBuf, quadRobj.mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    // Draw a fullscreen quad and composite the final image
    vkCmdDrawIndexed(cmdBuf, quadRobj.mesh->indexCount, 1, 0, 0, 0);

    postRenderPass.cmdEnd(cmdBuf);

    commandBuffers[currentImage].end();
}

/* Draws a frame and handles the update and synchronization of frames.
	CPU GPU sync is done using fences
	GPU GPU sync is done using semaphores
*/
void TestRenderer::drawFrame() {
    // Wait for the old frame to finish rendering
    vkWaitForFences(device.getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // Aquire the next image to draw to from the swapchain
    // Specifies the sync objects to be notified when the image is ready
    uint32_t imageIndex; // index of available image
    VkResult result = vkAcquireNextImageKHR(device.getDevice(),
                                            swapChain.vSwapChain(), UINT64_MAX /*timeout*/,
                                            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) { // swap chain has been invalidated
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("VULKAN: failed to acquire swap chain image!");
    }

    // Update the uniforms and draw commands
    updateUniformBuffer(imageIndex);
    updateCommandBuffer(imageIndex);

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
    std::array<VkCommandBuffer, 1> activeCommandBuffers = {commandBuffers[imageIndex].getBuffer()};
    submitInfo.commandBufferCount = static_cast<uint32_t>(activeCommandBuffers.size());
    submitInfo.pCommandBuffers = activeCommandBuffers.data();
    // Specify semaphore to notify after finishing
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Reset the finish fence
    vkResetFences(device.getDevice(), 1, &inFlightFences[currentFrame]);

    // Submit the command buffers to the queue and the fence to be notified after finishing this frame
    if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to submit draw command buffer!");
    }

    // Present the frame on the screen
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // Wait for the frame to be finished
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    // Specify the swapchain for presentation
    VkSwapchainKHR swapChains[] = {swapChain.vSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    // Specify the image to present
    presentInfo.pImageIndices = &imageIndex;
    // Present the frame
    result = vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        window.getFrameBufferResize()) { // swapchain is invalid
        window.setFrameBufferResized(false);
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to present swap chain image!");
    }

    // Counter for current frame
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

/* Recreates the swap chain after e.g. a window resize. */
void TestRenderer::recreateSwapChain() {
    std::cout << "Recreating swap chain" << std::endl;

    // Get new dimensions
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window.getWindow(), &width, &height);
        glfwWaitEvents();
    }

    // Wait for current rendering to finish
    device.waitIdle();

    // Destroy old objects
    cleanupSwapChain();

    // Create new createSyncObjects
    swapChain.reinit();

    // Recreate the render pass attachments
    createDepthResources();
    createImageBuffers();

    // Recreate the render passes that depend on the swap chain
    mainGraphicsPass.recreate();
    imGuiRenderPass.recreate();
    postRenderPass.recreate();
    // Update the attachment views
    postRenderPass.setImageBufferViews(offscreenImageView, depthImageView, imGuiImageView);


    // Recreate the framebuffers for the render passes
    createFramebuffers();

    createCommandBuffers();
}

/* Cleans up all objects that depend on the swapchain and its images. */
void TestRenderer::cleanupSwapChain() {

    // The framebuffers depend on the image views of the swap chain
    for (auto framebuffer : swapChainFramebuffers) {
        VulkanFramebuffer::destroy(device, framebuffer);
    }

    // Destroy offscreen main scene framebuffer
    VulkanFramebuffer::destroy(device, offscreenFramebuffer);
    // Destroy offscreen main scene image buffer
    VulkanImage::destroy(device, offscreenImage, offscreenImageMemory);
    VulkanImageView::destroy(device, offscreenImageView);
    // Destroy offscreen main scene depth buffer
    VulkanImage::destroy(device, depthImage, depthImageMemory);
    VulkanImageView::destroy(device, depthImageView);

    // Destroy offscreen ImGui framebuffer
    VulkanFramebuffer::destroy(device, imGuiFramebuffer);
    // Destroy offscreen ImGui image buffer
    VulkanImage::destroy(device, imGuiImage, imGuiImageMemory);
    VulkanImageView::destroy(device, imGuiImageView);

    // The command buffers depend on the amount of swapchain images
    for (auto cmdBuffer : commandBuffers) {
        cmdBuffer.destroy();
    }

    mainGraphicsPass.destroySwapChainDependent();
    imGuiRenderPass.destroySwapChainDependent();
    postRenderPass.destroySwapChainDependent();

    swapChain.destroy();
}

/* Cleans up all Vulkan objects in the proper order. */
void TestRenderer::destroyResources() {
    cleanupSwapChain();

    // Finish cleaning up the render passes, partly done in cleanupSwapChain()
    mainGraphicsPass.destroy();
    imGuiRenderPass.destroy();
    postRenderPass.destroy();

    // Free the model buffers
    for (auto &m : meshes) {
        vulkanMemory.destroy(m.vertexBuffer);
        vulkanMemory.destroy(m.indexBuffer);
    }

    // Destroy the synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device.getDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device.getDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device.getDevice(), inFlightFences[i], nullptr);
    }

    // Cleanup all other allocations
    vulkanMemory.destroy();

    // Destroy the command pool
    vkDestroyCommandPool(device.getDevice(), commandPool, nullptr);
}

// Data management

RenderMesh TestRenderer::uploadMesh(Mesh &mesh) {
    VulkanBuffer vertexBuffer = vulkanMemory.createInputBuffer(
            mesh.vertices.size() * sizeof(mesh.vertices[0]), mesh.vertices.data(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VulkanBuffer indexBuffer = vulkanMemory.createInputBuffer(
            mesh.indices.size() * sizeof(mesh.indices[0]), mesh.indices.data(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    RenderMesh rmesh{.vertexBuffer=vertexBuffer, .indexBuffer=indexBuffer,
            .indexCount=static_cast<uint32_t>(mesh.indices.size())};
    meshes.emplace_back(rmesh);
    return rmesh;
}

RenderObjectRef TestRenderer::addObject(RenderMesh &rmesh, glm::mat4 modelMat, MaterialRef material) {
    renderObjects.emplace_back(RenderObject{.mesh=&rmesh, .modelMat=modelMat, .material=material});
    return renderObjects.size() - 1;
}

bool TestRenderer::setModelMatrix(uint32_t robjID, glm::mat4 modelMat) {
    if (robjID >= renderObjects.size())
        return false;

    renderObjects[robjID].modelMat = modelMat;
    return true;
}

/* Creates a material containing only one texture and loads the texture if it
	has not been loaded before. 
	*/
MaterialRef TestRenderer::createMaterial(const TexturePhongMaterial &material) {
    return mainGraphicsPass.createMaterial(material);
}


void TestRenderer::setViewMatrix(const glm::mat4 &view) {
    camera.view = view;
}

void TestRenderer::setCameraAngle(float angle) {
    camera.angle = angle;
}

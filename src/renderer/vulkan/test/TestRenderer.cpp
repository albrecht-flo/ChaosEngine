#include "TestRenderer.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstring>

#include "src/renderer/data/ModelLoader.h"
#include "src/renderer/vulkan/command/VulkanCommandPool.h"

TestRenderer::TestRenderer(Window &w) :
        VulkanRendererOld(w),
        swapChain(VulkanSwapChain::Create(window, device, surface)),
        commandPool(VulkanCommandPool::Create(device)),
        vulkanMemory(device, commandPool),
        mainGraphicsPass(device, vulkanMemory, swapChain),
        imGuiRenderPass(device, vulkanMemory, swapChain, window, instance),
        postRenderPass(device, vulkanMemory, swapChain),
        offscreenImageView(device),
        depthBuffer(device),
        imGuiImageView(device),
        offscreenFramebuffer(device),
        imGuiFramebuffer(device) {}

void TestRenderer::init() {
    // Vulkan Instance and Device are handled by VulkanRendererOld constructor

    // GPU communication
    // Create per frame command buffers
    createCommandBuffers();
    // Sync objects for drawing frames
    createSyncObjects();

    // requires memory manager for depth image creation
    createDepthResources();
    createImageBuffers();

    // Render rendering
    mainGraphicsPass.init();
    imGuiRenderPass.init();
    postRenderPass.init();
    postRenderPass.setImageBufferViews(offscreenImageView.vk(), depthBuffer.getImageView().vk(), imGuiImageView.vk());

    createFramebuffers();

    auto quad = ModelLoader::getQuad();
    quadMesh = uploadMesh(quad);
    quadRobj.mesh = &quadMesh;
    quadRobj.modelMat = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, -3.0f));
}


/* Creates the command buffers for all frames from the command pool. */
void TestRenderer::createCommandBuffers() {
    primaryCommandBuffers.clear();
    primaryCommandBuffers.reserve(swapChain.size());

    for (size_t i = 0; i < swapChain.size(); i++) {
        primaryCommandBuffers.emplace_back(VulkanCommandBuffer::Create(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY));
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
        if (vkCreateSemaphore(device.vk(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) !=
            VK_SUCCESS ||
            vkCreateSemaphore(device.vk(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) !=
            VK_SUCCESS ||
            vkCreateFence(device.vk(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("[Vulkan] Failed to create synchronization objects for a frame!");
        }
    }
}

/* Creates the depth resources for depth buffering. */
void TestRenderer::createDepthResources() {
    VkFormat depthFormat = VulkanImage::getDepthFormat(device);

    VkDeviceMemory depthImageMemory{};
    auto depthImage = VulkanImage::createDepthBufferImage(
            device, vulkanMemory, swapChain.getExtent().width,
            swapChain.getExtent().height,
            depthFormat, depthImageMemory);
    auto depthImageView = VulkanImageView::Create(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    depthBuffer = VulkanImageBuffer(device, std::move(depthImage), std::move(depthImageMemory),
                                    std::move(depthImageView), swapChain.getWidth(), swapChain.getHeight());
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
            VulkanImageView::Create(device,
                                    offscreenImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

    // Offscreen ImGui rendering
    imGuiImage =
            VulkanImage::createRawImage(device,
                                        vulkanMemory, swapChain.getExtent().width, swapChain.getExtent().height,
                                        VK_FORMAT_R8G8B8A8_UNORM,
                                        imGuiImageMemory);
    imGuiImageView =
            VulkanImageView::Create(device,
                                    imGuiImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

/* Creates framebuffers for offscreen render passes and final composite rendering. */
void TestRenderer::createFramebuffers() {
    // Create offscreen frame buffer // ONLY ONE because we only have one frame in active rendering
    offscreenFramebuffer = VulkanFramebuffer::createFramebuffer(
            device,
            {offscreenImageView.vk(), depthBuffer.getImageView().vk()},
            mainGraphicsPass.vk(),
            swapChain.getExtent().width, swapChain.getExtent().height
    );
    // Create offscreen ImGui framebuffer
    imGuiFramebuffer = VulkanFramebuffer::createFramebuffer(
            device,
            {imGuiImageView.vk()},
            imGuiRenderPass.vk(),
            swapChain.getExtent().width, swapChain.getExtent().height
    );

    // Create framebuffers for swap chain
    swapChainFramebuffers.clear();
    swapChainFramebuffers.reserve(swapChain.size());
    for (uint32_t i = 0; i < swapChain.size(); i++) {
        swapChainFramebuffers.emplace_back(VulkanFramebuffer::createFramebuffer(
                device,
                {swapChain.getImageViews()[i].vk()},
                postRenderPass.vk(),
                swapChain.getExtent().width, swapChain.getExtent().height
        ));
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
    primaryCommandBuffers[currentImage].begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    VkCommandBuffer cmdBuf = primaryCommandBuffers[currentImage].vk();

    // Let the render rendering setup context descriptors
    mainGraphicsPass.cmdBegin(cmdBuf, currentImage, offscreenFramebuffer.vk(), swapChain.getWidth(),
                              swapChain.getHeight());

    for (auto &robj : renderObjects) {
        // Let the render rendering setup per model descriptors
        mainGraphicsPass.cmdRender(cmdBuf, robj);

        // Bind the vertex buffer
        VkBuffer vertexBuffers[]{robj.mesh->vertexBuffer.buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmdBuf, 0, 1, vertexBuffers, offsets);
        // Bind the index buffer
        vkCmdBindIndexBuffer(cmdBuf, robj.mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmdBuf, robj.mesh->indexCount, 1, 0, 0, 0);
    }

    // End this render rendering
    mainGraphicsPass.cmdEnd(cmdBuf);

    // !!! Transition of image layouts performed by attachment final layouts
    // !!! synchronization done by mainGraphicsPass->dependency.dstSubPass EXTERNAL
    // !!!							imGuiRenderPass->dependency.srcSubPass EXTERNAL

    imGuiRenderPass.cmdBegin(cmdBuf, currentImage, imGuiFramebuffer.vk());
    // No more needed, ImGui rendering is called in cmdBegin
    imGuiRenderPass.cmdEnd(cmdBuf);

    // !!! Transition of image layouts performed by attachment final layouts
    // !!! synchronization done by imGuiRenderPass->dependency.dstSubPass EXTERNAL
    // !!!							postRenderPass->dependency.srcSubPass EXTERNAL

    // Start post processing render rendering
    postRenderPass.cmdBegin(cmdBuf, currentImage, swapChainFramebuffers[currentImage].vk(), swapChain.getWidth(),
                            swapChain.getHeight());
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

    primaryCommandBuffers[currentImage].end();
}

/* Draws a frame and handles the update and synchronization of frames.
	CPU GPU sync is done using fences
	GPU GPU sync is done using semaphores
*/
void TestRenderer::drawFrame() {
    // Wait for the old frame to finish rendering
    vkWaitForFences(device.vk(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // Aquire the next image to draw to from the swapchain
    // Specifies the sync objects to be notified when the image is ready
    uint32_t imageIndex; // index of available image
    VkResult result = vkAcquireNextImageKHR(device.vk(),
                                            swapChain.getSwapChain(), UINT64_MAX /*timeout*/,
                                            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) { // swap chain has been invalidated
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("[Vulkan] Failed to acquire swap chain image!");
    }

    // Update the uniforms and draw commands
    updateUniformBuffer(imageIndex);
    updateCommandBuffer(imageIndex);

    // Prepare to submit the command buffers for the current frame
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]}; // wait for the image to be available
    VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT}; // wait for the image to be writeable before writing the color output
    // This means that the vertex shader stage etc. can already be executed
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores; // semaphore to wait for before executing
    submitInfo.pWaitDstStageMask = waitStages; // stages to wait for before executing
    // Specify the command buffers to submit for this draw call
    std::array<VkCommandBuffer, 1> activeCommandBuffers = {primaryCommandBuffers[imageIndex].vk()};
    submitInfo.commandBufferCount = static_cast<uint32_t>(activeCommandBuffers.size());
    submitInfo.pCommandBuffers = activeCommandBuffers.data();
    // Specify semaphore to notify after finishing
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Reset the finish fence
    vkResetFences(device.vk(), 1, &inFlightFences[currentFrame]);

    // Submit the command buffers to the queue and the fence to be notified after finishing this frame
    if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("[Vulkan] Failed to submit draw command buffer!");
    }

    // Present the frame on the screen
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // Wait for the frame to be finished
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    // Specify the swapchain for presentation
    VkSwapchainKHR swapChains[] = {swapChain.getSwapChain()};
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
        throw std::runtime_error("[Vulkan] Failed to present swap chain image!");
    }

    // Counter for current frame
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

/* Recreates the swap chain after e.g. a window resize. */
void TestRenderer::recreateSwapChain() {
    std::cout << "Recreating swap chain" << std::endl;

    // Wait for current rendering to finish
    device.waitIdle();

    // Destroy old objects
    cleanupSwapChain();

    // The framebuffers depend on the image views of the swap chain
    swapChainFramebuffers.clear();
    // Create new createSyncObjects
    surface = window.createSurface(instance.vk());
    swapChain.recreate(surface);

    // Recreate the render rendering attachments
    createDepthResources();
    createImageBuffers();

    // Recreate the render passes that depend on the swap chain
    mainGraphicsPass.recreate();
    imGuiRenderPass.recreate();
    postRenderPass.recreate();
    // Update the attachment views
    postRenderPass.setImageBufferViews(offscreenImageView.vk(), depthBuffer.getImageView().vk(), imGuiImageView.vk());


    // Recreate the framebuffers for the render passes (Destroys the old ones)
    createFramebuffers();

    createCommandBuffers();
}

/* Cleans up all objects that depend on the swapchain and its images. */
void TestRenderer::cleanupSwapChain() {

    // Destroy offscreen main scene image buffer
    VulkanImage::destroy(device, offscreenImage, offscreenImageMemory);
    // Destroy offscreen ImGui image buffer
    VulkanImage::destroy(device, imGuiImage, imGuiImageMemory);

    // The command buffers depend on the amount of swapchain images
    for (auto &cmdBuffer : primaryCommandBuffers) {
        cmdBuffer.destroy();
    }

    mainGraphicsPass.destroySwapChainDependent();
    imGuiRenderPass.destroySwapChainDependent();
    postRenderPass.destroySwapChainDependent();

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
        vkDestroySemaphore(device.vk(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device.vk(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device.vk(), inFlightFences[i], nullptr);
    }

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
    camera.fieldOfView = angle;
}

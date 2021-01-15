#include "TestRenderer.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

#include "../data/ModelLoader.h"

TestRenderer::TestRenderer(Window &w) :
        VulkanRenderer(w) {
}

TestRenderer::~TestRenderer() {}

void TestRenderer::init() {
    // Vulkan Instance and Device are handled by VulkanRenderer constructor

    // Swap chain creation
    m_swapChain.init(&m_device, &m_window, &m_surface);

    // GPU communication
    createCommandPool();
    // Create per frame command buffers
    createCommandBuffers();
    // Sync objects for draing frames
    createSyncObjects();

    // Initialize memory manager
    m_vulkanMemory = std::unique_ptr<VulkanMemory>(
            new VulkanMemory(m_device, m_commandPool));

    // requires memory manager for depth image creation
    createDepthResources();
    createImageBuffers();

    // Render pass
    m_mainGraphicsPass = MainSceneRenderPass(&m_device, m_vulkanMemory.get(), &m_swapChain);
    m_imGuiRenderPass = ImGuiRenderPass(&m_device, m_vulkanMemory.get(), &m_swapChain, &m_window);
    m_postRenderPass = PostRenderPass(&m_device, m_vulkanMemory.get(), &m_swapChain);
    m_postRenderPass.setImageBufferViews(m_offscreenImageView, m_depthImageView, m_imGuiImageView);

    createFramebuffers();

    Mesh *quad = ModelLoader::getQuad();
    // createMaterial(TexturePhongMaterial{"Waves.jpg", 64.0f});
    m_quadMesh = uploadMesh(*quad);
    m_quadRobj.mesh = &m_quadMesh;
    m_quadRobj.modelMat = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, -3.0f));
}


/* Creates command pool to contain comman buffers */
void TestRenderer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = m_device.findQueueFamilies();

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // We want the buffers to be able to reset them
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_device.getDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to create graphics command pool!");
    }
}

/* Creates the command buffers for all frames from the command pool. */
void TestRenderer::createCommandBuffers() {
    m_commandBuffers.clear();
    m_commandBuffers.reserve(m_swapChain.size());

    for (size_t i = 0; i < m_swapChain.size(); i++) {
        m_commandBuffers.emplace_back(VulkanCommandBuffer(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY));
    }
}

/* Creates the objects for synchronization between frames. */
void TestRenderer::createSyncObjects() {
    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(m_device.getDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) !=
            VK_SUCCESS ||
            vkCreateSemaphore(m_device.getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) !=
            VK_SUCCESS ||
            vkCreateFence(m_device.getDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("VULKAN: failed to create synchronization objects for a frame!");
        }
    }
}

/* Creates the depth resources for depth buffering. */
void TestRenderer::createDepthResources() {
    VkFormat depthFormat = VulkanImage::getDepthFormat(m_device);

    m_depthImage = VulkanImage::createDepthBufferImage(
            m_device, *m_vulkanMemory, m_swapChain.getExtent().width,
            m_swapChain.getExtent().height,
            depthFormat, m_depthImageMemory);
    m_depthImageView = VulkanImageView::create(m_device, m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

/* Creates images and imageviews for offscreen buffers. */
void TestRenderer::createImageBuffers() {
    // Offscreen main scene rendering
    m_offscreenImage =
            VulkanImage::createRawImage(m_device,
                                        *m_vulkanMemory, m_swapChain.getExtent().width, m_swapChain.getExtent().height,
                                        VK_FORMAT_R8G8B8A8_UNORM,
                                        m_offscreenImageMemory);
    m_offscreenImageView =
            VulkanImageView::create(m_device,
                                    m_offscreenImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

    // Offscreen ImGui rendering
    m_imGuiImage =
            VulkanImage::createRawImage(m_device,
                                        *m_vulkanMemory, m_swapChain.getExtent().width, m_swapChain.getExtent().height,
                                        VK_FORMAT_R8G8B8A8_UNORM,
                                        m_imGuiImageMemory);
    m_imGuiImageView =
            VulkanImageView::create(m_device,
                                    m_imGuiImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

/* Creates framebuffers for offscreen render passes and final composit pass. */
void TestRenderer::createFramebuffers() {
    // Create offscreen frame buffer // ONLY ONE because we only have one frame in active rendering
    m_offscreenFramebuffer = VulkanFramebuffer::createFramebuffer(
            m_device,
            {m_offscreenImageView, m_depthImageView},
            m_mainGraphicsPass.getVkRenderPass(),
            m_swapChain.getExtent().width, m_swapChain.getExtent().height
    );
    // Create offscreen ImGui framebuffer
    m_imGuiFramebuffer = VulkanFramebuffer::createFramebuffer(
            m_device,
            {m_imGuiImageView},
            m_imGuiRenderPass.getVkRenderPass(),
            m_swapChain.getExtent().width, m_swapChain.getExtent().height
    );

    // Create framebuffers for swap chain
    m_swapChainFramebuffers.resize(m_swapChain.size());
    for (uint32_t i = 0; i < m_swapChain.size(); i++) {
        m_swapChainFramebuffers[i] = VulkanFramebuffer::createFramebuffer(
                m_device,
                {m_swapChain.getImageViews()[i]},
                m_postRenderPass.getVkRenderPass(),
                m_swapChain.getExtent().width, m_swapChain.getExtent().height
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
    m_mainGraphicsPass.updateUniformBuffer(currentImage, m_camera, worldLight);
    m_postRenderPass.updateCamera(m_camera);
}

void TestRenderer::updateCommandBuffer(uint32_t currentImage) {
    // Implicitly resets the command buffer.
    m_commandBuffers[currentImage].begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    VkCommandBuffer cmdBuf = m_commandBuffers[currentImage].getBuffer();

    // Let the render pass setup general descriptors
    m_mainGraphicsPass.cmdBegin(cmdBuf, currentImage, m_offscreenFramebuffer);

    for (size_t i = 0; i < m_renderObjects.size(); i++) {
        RenderObject &robj = m_renderObjects[i];
        // Let the render pass setup per model descriptors
        m_mainGraphicsPass.cmdRender(cmdBuf, robj);

        // Bind the vertex buffer
        VkBuffer vertexBuffers[]{robj.mesh->vertexBuffer.buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmdBuf, 0, 1, vertexBuffers, offsets);
        // Bind the index buffer
        vkCmdBindIndexBuffer(cmdBuf, robj.mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmdBuf, robj.mesh->indexCount, 1, 0, 0, 0);
    }

    // End this render pass
    m_mainGraphicsPass.cmdEnd(cmdBuf);

    // !!! Transition of image layouts performed by attachment final layouts
    // !!! synchronization done by m_mainGraphicsPass->dependency.dstSubPass EXTERNAL
    // !!!							m_imGuiRenderPass->dependency.srcSubPass EXTERNAL

    m_imGuiRenderPass.cmdBegin(cmdBuf, currentImage, m_imGuiFramebuffer);
    // No more needed, ImGui rendering is called in cmdBegin
    m_imGuiRenderPass.cmdEnd(cmdBuf);

    // !!! Transition of image layouts performed by attachment final layouts
    // !!! synchronization done by m_imGuiRenderPass->dependency.dstSubPass EXTERNAL
    // !!!							m_postRenderPass->dependency.srcSubPass EXTERNAL

    // Start post processing render pass
    m_postRenderPass.cmdBegin(cmdBuf, currentImage, m_swapChainFramebuffers[currentImage]);
    // Bind previous color attachment to fragment shader sampler
    m_postRenderPass.cmdRender(cmdBuf, m_quadRobj);
    // Bind a screen filling quad to render the previous render passes to
    VkBuffer vertexBuffers[]{m_quadRobj.mesh->vertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmdBuf, m_quadRobj.mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    // Draw a fullscreen quad and composit the final image
    vkCmdDrawIndexed(cmdBuf, m_quadRobj.mesh->indexCount, 1, 0, 0, 0);

    m_postRenderPass.cmdEnd(cmdBuf);

    m_commandBuffers[currentImage].end();
}

/* Draws a frame and handles the update and synchronization of frames.
	CPU GPU sync is done using fences
	GPU GPU sync is done using semaphores
*/
void TestRenderer::drawFrame() {
    // Wait for the old frame to finish rendering
    vkWaitForFences(m_device.getDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    // Aquire the next image to draw to from the swapchain
    // Specifies the sync objects to be notified when the image is ready
    uint32_t imageIndex; // index of available image
    VkResult result = vkAcquireNextImageKHR(m_device.getDevice(),
                                            m_swapChain.vSwapChain(), UINT64_MAX /*timeout*/,
                                            m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

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

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]}; // wait for the image to be available
    VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // wait for the image to be writeable before writing the color output
    // This means that the vertex shader stage etc. can already be executed
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores; // semaphore to wait for before executing
    submitInfo.pWaitDstStageMask = waitStages; // stages to wait for before executing
    // Specify the command buffers to submit for this draw call
    std::array<VkCommandBuffer, 1> activeCommandBuffers = {m_commandBuffers[imageIndex].getBuffer()};
    submitInfo.commandBufferCount = static_cast<uint32_t>(activeCommandBuffers.size());
    submitInfo.pCommandBuffers = activeCommandBuffers.data();
    // Specify semaphore to notify after finishing
    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Reset the finish fence
    vkResetFences(m_device.getDevice(), 1, &m_inFlightFences[m_currentFrame]);

    // Submit the command buffers to the queue and the fence to be notified after finishing this frame
    if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to submit draw command buffer!");
    }

    // Present the frame on the screen
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // Wait for the frame to be finished
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    // Specify the swapchain for presentation
    VkSwapchainKHR swapChains[] = {m_swapChain.vSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    // Specify the image to present
    presentInfo.pImageIndices = &imageIndex;
    // Present the frame
    result = vkQueuePresentKHR(m_device.getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        m_window.getFrameBufferResize()) { // swapchain is invalid
        m_window.setFrameBufferResized(false);
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("VULKAN: failed to present swap chain image!");
    }

    // Counter for current frame
    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

/* Recreates the swap chain after e.g. a window resize. */
void TestRenderer::recreateSwapChain() {
    std::cout << "Recreating swap chain" << std::endl;

    // Get new dimensions
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window.getWindow(), &width, &height);
        glfwWaitEvents();
    }

    // Wait for current rendering to finish
    m_device.waitIdle();

    // Destroy old objects
    cleanupSwapChain();

    // Create new createSyncObjects
    m_swapChain.reinit();

    // Recreate the render pass attachments
    createDepthResources();
    createImageBuffers();

    // Recreate the render passes that depend on the swap chain
    m_mainGraphicsPass.recreate();
    m_imGuiRenderPass.recreate();
    m_postRenderPass.recreate();
    // Update the attachment views
    m_postRenderPass.setImageBufferViews(m_offscreenImageView, m_depthImageView, m_imGuiImageView);


    // Recreate the framebuffers for the render passes
    createFramebuffers();

    createCommandBuffers();
}

/* Cleans up all objects that depend on the swapchain and its images. */
void TestRenderer::cleanupSwapChain() {

    // The framebuffers depend on the image views of the swap chain
    for (auto framebuffer : m_swapChainFramebuffers) {
        VulkanFramebuffer::destroy(m_device, framebuffer);
    }

    // Destroy offscreen main scene framebuffer
    VulkanFramebuffer::destroy(m_device, m_offscreenFramebuffer);
    // Destroy offscreen main scene image buffer
    VulkanImage::destroy(m_device, m_offscreenImage, m_offscreenImageMemory);
    VulkanImageView::destroy(m_device, m_offscreenImageView);
    // Destroy offscreen main scene depth buffer
    VulkanImage::destroy(m_device, m_depthImage, m_depthImageMemory);
    VulkanImageView::destroy(m_device, m_depthImageView);

    // Destroy offscreen ImGui framebuffer
    VulkanFramebuffer::destroy(m_device, m_imGuiFramebuffer);
    // Destroy offscreen ImGui image buffer
    VulkanImage::destroy(m_device, m_imGuiImage, m_imGuiImageMemory);
    VulkanImageView::destroy(m_device, m_imGuiImageView);

    // The command buffers depend on the ammount of swapchain images
    for (auto cmdBuffer : m_commandBuffers) {
        cmdBuffer.destroy();
    }

    m_mainGraphicsPass.destroySwapChainDependent();
    m_imGuiRenderPass.destroySwapChainDependent();
    m_postRenderPass.destroySwapChainDependent();

    m_swapChain.destroy();
}

/* Cleans up all Vulkan objects in the propper order. */
void TestRenderer::destroyResources() {
    cleanupSwapChain();

    // Finish cleaning up the render passes, partly done in cleanupSwapChain()
    m_mainGraphicsPass.destroy();
    m_imGuiRenderPass.destroy();
    m_postRenderPass.destroy();

    // Free the model buffers
    for (auto &m : m_meshes) {
        m_vulkanMemory->destroy(m.vertexBuffer);
        m_vulkanMemory->destroy(m.indexBuffer);
    }

    // Destroy the synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_device.getDevice(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_device.getDevice(), m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_device.getDevice(), m_inFlightFences[i], nullptr);
    }

    // Cleanup all other allocations
    m_vulkanMemory->destroy();

    // Destroy the command pool
    vkDestroyCommandPool(m_device.getDevice(), m_commandPool, nullptr);
}

// Data management

RenderMesh TestRenderer::uploadMesh(Mesh &mesh) {
    VulkanBuffer vertexBuffer = m_vulkanMemory->createInputBuffer(
            mesh.vertices.size() * sizeof(mesh.vertices[0]), mesh.vertices.data(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VulkanBuffer indexBuffer = m_vulkanMemory->createInputBuffer(
            mesh.indices.size() * sizeof(mesh.indices[0]), mesh.indices.data(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    RenderMesh rmesh{.vertexBuffer=vertexBuffer, .indexBuffer=indexBuffer,
            .indexCount=static_cast<uint32_t>(mesh.indices.size())};
    m_meshes.emplace_back(rmesh);
    return rmesh;
}

RenderObjectRef TestRenderer::addObject(RenderMesh &rmesh, glm::mat4 modelMat, MaterialRef material) {
    m_renderObjects.emplace_back(RenderObject{.mesh=&rmesh, .modelMat=modelMat, .material=material});
    return m_renderObjects.size() - 1;
}

bool TestRenderer::setModelMatrix(uint32_t robjID, glm::mat4 modelMat) {
    if (robjID >= m_renderObjects.size())
        return false;

    m_renderObjects[robjID].modelMat = modelMat;
    return true;
}

/* Creates a material contianing only one texture and loads the texture if it
	has not been loaded before. 
	*/
MaterialRef TestRenderer::createMaterial(const TexturePhongMaterial material) {
    return m_mainGraphicsPass.createMaterial(material);
}


void TestRenderer::setViewMatrix(const glm::mat4 &view) {
    m_camera.view = view;
}

void TestRenderer::setCameraAngle(float angle) {
    m_camera.angle = angle;
}

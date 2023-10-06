set(ENGINE_SOURCES
        #       External vendor files
        src/vendor/vk_mem_allocator.cpp
        src/vendor/imgui.cpp
        src/vendor/stb.cpp
        #       Core classes
        src/core/Engine.cpp
        src/core/Scene.cpp
        src/core/Ecs.cpp
        src/core/Entity.cpp
        src/core/renderSystem/RenderingSystem.cpp
        src/core/renderSystem/UIRenderSubSystem.cpp
        src/core/assets/Mesh.cpp
        src/core/assets/ModelLoader.cpp
        src/core/assets/RawImage.cpp
        src/core/assets/AssetManager.cpp
        src/core/assets/AssetLoader.cpp
        src/core/assets/FontManager.cpp
        src/core/utils/Logger.cpp
        src/core/utils/STDExtensions.cpp
        src/core/scriptSystem/NativeScriptSystem.cpp
        src/core/scriptSystem/NativeScript.cpp
        src/core/uiSystem/UISystem.cpp
        src/core/assets/Font.cpp
        #     Engine API classes
        src/renderer/api/RenderMesh.cpp
        src/renderer/api/Buffer.cpp
        src/renderer/api/Material.cpp
        src/renderer/api/RenderPass.cpp
        src/renderer/api/Texture.cpp
        src/renderer/api/BufferedGPUResource.cpp
        src/renderer/api/Framebuffer.cpp
        src/renderer/api/GraphicsContext.cpp
        src/renderer/window/Window.cpp
        #       General Vulkan Rendering files
        src/renderer/passes/SpriteRenderingPass.cpp
        src/renderer/passes/UIRenderingPass.cpp
        src/renderer/passes/PostProcessingPass.cpp
        src/renderer/passes/ImGuiRenderingPass.cpp
        src/renderer/VulkanRenderer2D.cpp
        #       Vulkan Rendering files
        src/renderer/vulkan/api/VulkanMaterial.cpp
        src/renderer/vulkan/api/VulkanRenderMesh.cpp
        src/renderer/vulkan/rendering/VulkanFrame.cpp
        src/renderer/vulkan/rendering/VulkanRenderPass.cpp
        src/renderer/vulkan/rendering/VulkanAttachmentBuilder.cpp
        src/renderer/vulkan/context/VulkanContext.cpp
        src/renderer/vulkan/context/VulkanDevice.cpp
        src/renderer/vulkan/context/VulkanInstance.cpp
        src/renderer/vulkan/context/VulkanSwapChain.cpp
        src/renderer/vulkan/memory/VulkanMemory.cpp
        src/renderer/vulkan/memory/VulkanBuffer.cpp
        src/renderer/vulkan/command/VulkanCommandBuffer.cpp
        src/renderer/vulkan/command/VulkanCommandPool.cpp
        src/renderer/vulkan/image/VulkanImage.cpp
        src/renderer/vulkan/image/VulkanSampler.cpp
        src/renderer/vulkan/image/VulkanFramebuffer.cpp
        src/renderer/vulkan/image/VulkanImageView.cpp
        src/renderer/vulkan/image/VulkanTexture.cpp
        src/renderer/vulkan/pipeline/VulkanDescriptorPool.cpp
        src/renderer/vulkan/pipeline/VulkanDescriptorPoolBuilder.cpp
        src/renderer/vulkan/pipeline/VulkanDescriptorSet.cpp
        src/renderer/vulkan/pipeline/VulkanDescriptorSetLayout.cpp
        src/renderer/vulkan/pipeline/VulkanDescriptorSetLayoutBuilder.cpp
        src/renderer/vulkan/pipeline/VulkanPipeline.cpp
        src/renderer/vulkan/pipeline/VulkanPipelineBuilder.cpp
        src/renderer/vulkan/pipeline/VulkanPipelineLayout.cpp
        src/renderer/vulkan/pipeline/VulkanPipelineLayoutBuilder.cpp
        src/renderer/vulkan/pipeline/VulkanVertexInput.cpp
        #       Testing Rendering files
        src/renderer/testRenderer/TestContext.cpp
        src/renderer/testRenderer/TestRenderer.cpp
        src/renderer/testRenderer/TestRenderPass.cpp
        src/renderer/testRenderer/TestFramebuffe r.cpp
        src/renderer/testRenderer/TestTexture.cpp
        )

set(ADDITIONAL_INCLUDE_DIRS
        ../dep/glm
        ../dep/glfw/include
        ../dep/vulkan
        ../dep/VulkanMemoryAllocator/src
        ../dep/imgui
        ../dep/freetype/include
        ../dep/entt/src
        ../dep/tinyply/source
        ../dep/tinyobjloader
        ../dep/stb
        ../dep/spdlog/include
        src/
        )

add_library(Engine ${ENGINE_SOURCES})
target_include_directories(Engine PUBLIC ${CMAKE_SOURCE_DIR} ${ADDITIONAL_INCLUDE_DIRS})

if (WIN32)
    target_link_directories(Engine PUBLIC ../dep/lib/win)
    target_link_libraries(Engine vulkan-1 glfw3 freetype)
elseif (UNIX AND NOT APPLE)
    target_link_directories(Engine PUBLIC ../dep/lib/linux)
    target_link_libraries(Engine X11 dl pthread vulkan glfw3 freetype)
else ()
    message(SEND_ERROR, "Unsuported platform")
endif ()
add_compile_definitions(GLFW_INCLUDE_NONE)


message(STATUS "Configured Engine build")
message(STATUS "Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")

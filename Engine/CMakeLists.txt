set(ENGINE_SOURCES
        #       External vendor files
        src/vendor/imgui/imgui.cpp
        #       Core classes
        src/core/Engine.cpp
        src/core/Scene.cpp
        src/core/Ecs.cpp
        src/core/Entity.cpp
        #       General Rendering files
        src/renderer/DevRenderer.cpp
        src/renderer/api/GraphicsContext.cpp
        src/renderer/passes/SpriteRenderingPass.cpp
        src/renderer/passes/PostProcessingPass.cpp
        src/renderer/passes/ImGuiRenderingPass.cpp
        #       Vulkan Rendering files
        src/renderer/window/Window.cpp
        src/renderer/data/Mesh.cpp
        src/renderer/data/ModelLoader.cpp
        src/renderer/data/RenderObject.cpp
        src/renderer/VulkanRenderer2D.cpp
        src/renderer/vulkan/rendering/VulkanFrame.cpp
        src/renderer/vulkan/rendering/VulkanRenderPass.cpp
        src/renderer/vulkan/rendering/VulkanAttachmentBuilder.cpp
        src/renderer/vulkan/context/VulkanContext.cpp
        src/renderer/vulkan/context/VulkanDevice.cpp
        src/renderer/vulkan/context/VulkanInstance.cpp
        src/renderer/vulkan/context/VulkanSwapChain.cpp
        src/renderer/vulkan/memory/VulkanMemory.cpp
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
        #     Test classes
        src/renderer/vulkan/test/VulkanRendererOld.cpp
        src/renderer/vulkan/test/VulkanRenderPassOld.cpp
        src/renderer/vulkan/test/TestRenderer.cpp
        src/renderer/vulkan/test/MainSceneRenderPass.cpp
        src/renderer/vulkan/test/PostRenderPass.cpp
        src/renderer/vulkan/test/ImGuiRenderPass.cpp
        src/core/RenderingSystem.cpp
        )

set(ADDITIONAL_INCLUDE_DIRS
        ../dep/glm
        ../dep/glfw/include
        ../dep/vulkan
        ../dep/imgui
        ../dep/entt/src
        ../dep/tinyply/source
        ../dep/tinyobjloader
        ../dep/stb
        )

add_library(Engine ${ENGINE_SOURCES})
target_include_directories(Engine PUBLIC ${CMAKE_SOURCE_DIR} ${ADDITIONAL_INCLUDE_DIRS})

if (WIN32)
    target_link_directories(Engine PUBLIC ../dep/lib/win)
    target_link_libraries(Engine vulkan-1 glfw3)
elseif (UNIX AND NOT APPLE)
    target_link_directories(Engine PUBLIC ../dep/lib/linux)
    target_link_libraries(Engine X11 dl pthread ../dep/vulkan glfw3)
else ()
    message(SEND_ERROR, "Unsuported platform")
endif ()
add_compile_definitions(GLFW_INCLUDE_NONE)


message(STATUS "Configured Engine build")
message(STATUS "Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
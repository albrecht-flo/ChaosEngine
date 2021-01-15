set(RENDER_ENGINE_SOURCES
        renderer/window/Window.cpp
        renderer/VulkanRenderer.cpp
        renderer/VulkanRenderPass.cpp
        renderer/data/Mesh.cpp
        renderer/data/ModelLoader.cpp
        renderer/data/RenderObject.cpp
        renderer/data/VulkanTexture.cpp
        renderer/general/VulkanDevice.cpp
        renderer/general/VulkanInstance.cpp
        renderer/general/VulkanSwapChain.cpp
        renderer/general/VulkanCommandBuffer.cpp
        renderer/image/VulkanImage.cpp
        renderer/image/VulkanSampler.cpp
        renderer/image/VulkanFramebuffer.cpp
        renderer/image/VulkanImageView.cpp
        renderer/memory/VulkanMemory.cpp
        renderer/pipeline/VulkanDescriptor.cpp
        renderer/pipeline/VulkanPipeline.cpp
        renderer/test/TestRenderer.cpp
        renderer/test/MainSceneRenderPass.cpp
        renderer/test/PostRenderPass.cpp
        renderer/test/ImGuiRenderPass.cpp
        renderer/vendor/imgui/imgui.cpp
        renderer/vendor/imgui/imgui_impl_glfw.cpp
        renderer/vendor/imgui/imgui_impl_vulkan.cpp
        renderer/vendor/imgui/imgui_demo.cpp
        renderer/vendor/imgui/imgui_draw.cpp
        renderer/vendor/imgui/imgui_widgets.cpp
        renderer/vendor/imgui/imgui_tables.cpp
        )

set(ADDITIONAL_INCLUDE_DIRS
        ../dep/glm/
        ../dep/glfw/include
        ../dep/vulkan
        ../dep/imgui
        ../dep/tinyply/source
        ../dep/tinyobjloader
        ../dep/stb
        )

add_library(render_engine ${RENDER_ENGINE_SOURCES})
target_include_directories(render_engine PUBLIC ${CMAKE_SOURCE_DIR} ${ADDITIONAL_INCLUDE_DIRS})
target_link_directories(render_engine PUBLIC ../dep/lib/win)
target_link_libraries(render_engine vulkan-1 glfw3)

add_executable(application main.cpp)
target_link_libraries(application PUBLIC render_engine)


add_custom_command(
        TARGET application POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res/models
        ${CMAKE_CURRENT_BINARY_DIR}/models

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res/shaders
        ${CMAKE_CURRENT_BINARY_DIR}/shaders

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res/textures
        ${CMAKE_CURRENT_BINARY_DIR}/textures
)
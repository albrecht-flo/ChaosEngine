set(RENDER_ENGINE_SOURCES
        renderer/window/Window.cpp
        renderer/RenderAPI.cpp
        renderer/data/Mesh.cpp
        renderer/data/ModelLoader.cpp
        renderer/data/RenderObject.cpp
        renderer/vulkan/VulkanRenderer2D.cpp
        renderer/vulkan/VulkanDataManager.cpp
        renderer/vulkan/rendering/VulkanFrame.cpp
        renderer/vulkan/rendering/VulkanRenderPass.cpp
        renderer/vulkan/rendering/VulkanAttachmentBuilder.cpp
        renderer/vulkan/context/VulkanContext.cpp
        renderer/vulkan/context/VulkanDevice.cpp
        renderer/vulkan/context/VulkanInstance.cpp
        renderer/vulkan/context/VulkanSwapChain.cpp
        renderer/vulkan/memory/VulkanMemory.cpp
        renderer/vulkan/command/VulkanCommandBuffer.cpp
        renderer/vulkan/command/VulkanCommandPool.cpp
        renderer/vulkan/image/VulkanImage.cpp
        renderer/vulkan/image/VulkanSampler.cpp
        renderer/vulkan/image/VulkanFramebuffer.cpp
        renderer/vulkan/image/VulkanImageView.cpp
        renderer/vulkan/image/VulkanTexture.cpp
        renderer/vulkan/pipeline/VulkanVertexInput.cpp
        renderer/vulkan/pipeline/VulkanDescriptorSetLayout.cpp
        renderer/vulkan/pipeline/VulkanDescriptorSet.cpp
        renderer/vulkan/pipeline/VulkanPipelineBuilder.cpp
        renderer/vulkan/pipeline/VulkanPipeline.cpp
        renderer/vulkan/test/VulkanRendererOld.cpp
        renderer/vulkan/test/VulkanRenderPassOld.cpp
        renderer/vulkan/test/TestRenderer.cpp
        renderer/vulkan/test/MainSceneRenderPass.cpp
        renderer/vulkan/test/PostRenderPass.cpp
        renderer/vulkan/test/ImGuiRenderPass.cpp
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui_impl_glfw.cpp
        vendor/imgui/imgui_impl_vulkan.cpp
        vendor/imgui/imgui_demo.cpp
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/imgui_widgets.cpp
        vendor/imgui/imgui_tables.cpp

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

if (WIN32)
    target_link_directories(render_engine PUBLIC ../dep/lib/win)
    target_link_libraries(render_engine vulkan-1 glfw3)
elseif (UNIX AND NOT APPLE)
    target_link_directories(render_engine PUBLIC ../dep/lib/linux)
    target_link_libraries(render_engine X11 dl pthread vulkan glfw3)
else ()
    message(SEND_ERROR, "Unsuported platform")
endif ()
add_compile_definitions(GLFW_INCLUDE_NONE)

# ------------------------------------- application -----------------------------------------------

add_executable(application main.cpp)
target_link_libraries(application PUBLIC render_engine)


add_custom_command(
        TARGET application POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res/models
        ${CMAKE_CURRENT_BINARY_DIR}/models

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res/textures
        ${CMAKE_CURRENT_BINARY_DIR}/textures
)

# ------------------------------------- Shaders ---------------------------------------------------

set(GLSL_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../res/shaders/base.frag"
        "${CMAKE_CURRENT_SOURCE_DIR}/../res/shaders/base.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/../res/shaders/post.frag"
        "${CMAKE_CURRENT_SOURCE_DIR}/../res/shaders/post.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/../res/shaders/2DStaticSprite.frag"
        "${CMAKE_CURRENT_SOURCE_DIR}/../res/shaders/2DSprite.vert"
        )

find_program(GLSLC glslc)
foreach (GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSLC} -o ${SPIRV} ${GLSL}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)


add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)
add_dependencies(render_engine Shaders)

# ------------------------------------- Setup -----------------------------------------------------
add_custom_target(
        runBeforeAll
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders/"
)
add_dependencies(Shaders runBeforeAll)

message(STATUS "Configured application build")
message(STATUS "Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")

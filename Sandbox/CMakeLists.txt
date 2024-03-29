set(SANDBOX_SOURCES
        src/main.cpp
        src/common/AssetView.cpp
        src/common/CustomImGui.cpp
        src/test/EmptyScene.cpp
        src/test/TestScene.cpp
        src/editor/EditorScene.cpp
        src/editor/EditorBaseAssets.cpp
        src/editor/DefaultProject.cpp
        src/editor/EditorComponentUI.cpp
        src/editor/EditorComponents.cpp
        src/editor/EditorAssetSelector.cpp
        src/editor/EditorAssetManager.cpp
        src/editor/scripts/EditorCameraScript.cpp
        src/editor/scripts/BaseMovementScript.cpp
        )

set(ADDITIONAL_INCLUDE_DIRS
        ../Engine/src
        ../dep/glm
        ../dep/imgui
        )

add_executable(Sandbox ${SANDBOX_SOURCES})
add_dependencies(Sandbox Engine)

target_include_directories(Sandbox PUBLIC ${CMAKE_SOURCE_DIR} ${ADDITIONAL_INCLUDE_DIRS})
target_link_libraries(Sandbox PUBLIC Engine)

target_link_libraries(Sandbox PRIVATE nfd)

add_custom_command(
        TARGET Sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/models
        ${CMAKE_CURRENT_BINARY_DIR}/models

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/textures
        ${CMAKE_CURRENT_BINARY_DIR}/textures

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/fonts
        ${CMAKE_CURRENT_BINARY_DIR}/fonts
)

# ------------------------------------- Shaders ---------------------------------------------------
find_package(Vulkan REQUIRED)

set(GLSL_SOURCE_FILES
        "res/shaders/base.vert"
        "res/shaders/base.frag"
        "res/shaders/post.vert"
        "res/shaders/post.frag"
        "res/shaders/ENGINE_2DBase.vert"
        "res/shaders/ENGINE_2DBase.frag"
        "res/shaders/ENGINE_UIBase.vert"
        "res/shaders/ENGINE_UIBase.frag"
        "res/shaders/ENGINE_UIText.frag"
        "res/shaders/UI.frag"
        "res/shaders/2DDebug.vert"
        "res/shaders/2DSprite.vert"
        "res/shaders/2DStaticColoredSprite.frag"
        "res/shaders/ENGINE_2DPostProcessing.vert"
        "res/shaders/ENGINE_2DPostProcessing.frag"
        "res/shaders/2DStaticTexturedSprite.frag"
        )

set(GLSLC ${Vulkan_GLSLC_EXECUTABLE})
foreach (GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSLC} -o ${SPIRV} "${CMAKE_CURRENT_SOURCE_DIR}/${GLSL}"
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)


add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)
add_dependencies(Sandbox Shaders)

# ------------------------------------- Setup -----------------------------------------------------

add_custom_target(
        runBeforeAll
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders/"
)
add_dependencies(Shaders runBeforeAll)

message(STATUS "Configured sandbox build")
message(STATUS "Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
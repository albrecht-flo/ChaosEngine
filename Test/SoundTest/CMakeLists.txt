set(SoundTest_SOURCES
        src/main.cpp
        src/SoundTestScene.cpp
        src/CameraScript.cpp
        src/SoundButtonScript.cpp
        )

set(ADDITIONAL_INCLUDE_DIRS
        ../../Engine/src
        ../../dep/glm
        ../../dep/imgui
        )

add_executable(SoundTest ${SoundTest_SOURCES})
add_dependencies(SoundTest Engine)

target_include_directories(SoundTest PUBLIC ${CMAKE_SOURCE_DIR} ${ADDITIONAL_INCLUDE_DIRS})
target_link_libraries(SoundTest PUBLIC Engine)

target_link_libraries(SoundTest PRIVATE nfd)

add_custom_command(
        TARGET SoundTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/models
        ${CMAKE_CURRENT_BINARY_DIR}/models

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/textures
        ${CMAKE_CURRENT_BINARY_DIR}/textures

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/fonts
        ${CMAKE_CURRENT_BINARY_DIR}/fonts

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/sounds
        ${CMAKE_CURRENT_BINARY_DIR}/sounds
)

# ------------------------------------- Shaders ---------------------------------------------------
find_package(Vulkan REQUIRED)

set(GLSL_SOURCE_FILES
        "res/shaders/ENGINE_2DBase.vert"
        "res/shaders/ENGINE_2DBase.frag"
        "res/shaders/ENGINE_DEBUG.vert"
        "res/shaders/ENGINE_DEBUG.frag"
        "res/shaders/ENGINE_UIBase.vert"
        "res/shaders/ENGINE_UIBase.frag"
        "res/shaders/ENGINE_UIText.frag"
        "res/shaders/ENGINE_2DPostProcessing.vert"
        "res/shaders/ENGINE_2DPostProcessing.frag"
        "res/shaders/2DSprite.vert"
        "res/shaders/2DStaticTexturedSprite.frag"
        "res/shaders/UI.frag"
        )

set(GLSLC ${Vulkan_GLSLC_EXECUTABLE})
message(STATUS "Found glslc at ${GLSLC}")
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
        SoundTestShaders
        DEPENDS ${SPIRV_BINARY_FILES}
)
add_dependencies(SoundTest SoundTestShaders)

# ------------------------------------- Setup -----------------------------------------------------

add_custom_target(
        SoundTestRunBeforeAll
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders/"
)
add_dependencies(SoundTestShaders SoundTestRunBeforeAll)

message(STATUS "Configured SoundTest build")
message(STATUS "Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
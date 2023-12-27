set(BOX2DTEST_SOURCES
        src/main.cpp
        src/Box2DTestScene.cpp
        src/JumperScript.cpp
        src/PusherScript.cpp
        src/CameraScript.cpp
        )

set(ADDITIONAL_INCLUDE_DIRS
        ../../Engine/src
        ../../dep/glm
        ../../dep/imgui
        )

add_executable(Box2DTest ${BOX2DTEST_SOURCES})
add_dependencies(Box2DTest Engine)

target_include_directories(Box2DTest PUBLIC ${CMAKE_SOURCE_DIR} ${ADDITIONAL_INCLUDE_DIRS})
target_link_libraries(Box2DTest PUBLIC Engine)

target_link_libraries(Box2DTest PRIVATE nfd)

add_custom_command(
        TARGET Box2DTest POST_BUILD
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
        "res/shaders/2DPostProcessing.vert"
        "res/shaders/2DPostProcessing.frag"
        "res/shaders/2DStaticColoredSprite.frag"
        "res/shaders/2DStaticTexturedSprite.frag"
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
        Box2DTestShaders
        DEPENDS ${SPIRV_BINARY_FILES}
)
add_dependencies(Box2DTest Box2DTestShaders)

# ------------------------------------- Setup -----------------------------------------------------

add_custom_target(
        Box2DTestRunBeforeAll
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders/"
)
add_dependencies(Box2DTestShaders Box2DTestRunBeforeAll)

message(STATUS "Configured Box2DTest build")
message(STATUS "Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
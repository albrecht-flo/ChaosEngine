
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan is using 0 to 1

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <exception>
#include <iostream>
#include <chrono>
#include <src/renderer/RendererAPI.h>


#include "renderer/window/Window.h"
#include "renderer/vulkan/test/TestRenderer.h"
#include "renderer/data/Mesh.h"
#include "renderer/data/RenderObject.h"
#include "renderer/data/ModelLoader.h"

/* TODOs:
 *
	Current:
        - Proper logging

		- Learn about image layouts
		- layout transitions
		- pipeline Parameters

		- add tinyobjloader library
			x model loading
			- material loading
		- Group pipeline and Descriptor layout stuff into super class
			-> extend this to create different shaders
		- renderobjects by material / reuse material 
			-> DescriptorSet management

    To Think:
		- use factory Create() instead of init (https://abseil.io/tips/42)
        - Consider PIMPL pattern https://oliora.github.io/2015/12/29/pimpl-and-rule-of-zero.html

	Refactor:
		- Migrate to C++ headers
		- Memory management
			- https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
		- command buffers
			x Own class
			- Manager
		- Rendering
			- Render Objects (created from mesh)
				- Object manager within renderer
				- Map mapping renderObjectID -> {vertexBuffer, indexBuffer, pipeline}
				- Instance {renderObjectID, modelMat, materialDescriptorset}
			- Uniforms
				- push constants for per instance stuff (model matrix)
				- Descriptor sets for multi instance stuff (material) -> do not change often
				- Descriptor sets for multi object (camera) -> changes often = buffered, TODO constantly mapped
				- Descriptor sets for multi object (lights) -> changes not often = buffer switch
			- Textures
        - Render Passes
            - attachment handling
            - sub pass dependencies
		- Pipeline handling
			- Shaders
			- Pipeline
			- RenderPass
		- Swap chain
			- Cleanup
			- Creation -> Parameters

	Features:
		General:
		- ECS (entt or self implemented, probably switchable)
		- Resource management
			- Model loading
			- Texture loading
		- Noise functions
		Rendering:
		- Sky box
		- Transparent Pass (Alpha blending)
		- Lighting
		- GUI with ImGui
		- Post processing
		- Batch rendering
		- Geometry shader
		- Tesselation shader
		- Text rendering
		- Particles
		- Shadows
		- Reflections
		- PBR
*/

void run(Window &window, TestRenderer &renderer, ModelLoader &modelLoader) {
    // Setup models
    auto mesh = ModelLoader::loadMeshFromOBJ("models/monkey.obj");
    auto island = ModelLoader::loadMeshFromPLY("models/Low_Poly_island.ply");
    auto islandWater = ModelLoader::loadMeshFromPLY("models/Low_Poly_island_water.ply");
    if (!mesh || !island || !islandWater) {
        throw std::runtime_error("Unabel to load meshes.");
    }

    RenderMesh rmesh = renderer.uploadMesh(**mesh);
    RenderMesh islandRMesh = renderer.uploadMesh(**island);
    RenderMesh islandWaterRMesh = renderer.uploadMesh(**islandWater);
    // Setup materials
    TexturePhongMaterial testMaterial1 = {
            .textureFile = "noTex.jpg",
            .shininess = 128.0f
    };

    TexturePhongMaterial testMaterial2 = {
            .textureFile = "Plasma.jpg",
            .shininess = 32.0f
    };

    MaterialRef material1 = renderer.createMaterial(testMaterial1);
    MaterialRef material2 = renderer.createMaterial(testMaterial2);

    // Create RenderObjects
    glm::mat4 modelMat1 = glm::translate(glm::mat4(1.0f), glm::vec3(0, +0.0f, -4.0f));
    modelMat1 = glm::scale(modelMat1, glm::vec3(0.5f, 0.5f, 0.5f));
    RenderObjectRef robj1 = renderer.addObject(islandRMesh, modelMat1, material1);
    RenderObjectRef robj2 = renderer.addObject(islandWaterRMesh, modelMat1, material1);

    glm::mat4 modelMat3Pos = glm::translate(glm::mat4(1.0f), glm::vec3(+0.0, +2.0f, -2.0f));
    glm::mat4 modelMat3 = glm::scale(modelMat3Pos, glm::vec3(0.25f, 0.25f, 0.25f));
    RenderObjectRef robj3 = renderer.addObject(rmesh, modelMat3, material2);

    // FPS counter
    auto startTime = std::chrono::high_resolution_clock::now();
    uint32_t frameCounter = 0;
    auto delatTimer = startTime;
    float fpsDelta = 0;
    // Camera points
    glm::vec3 origin = glm::vec3(0, 0, +0.0f);
    glm::vec3 target = glm::vec3(0, 0, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0, 1.0f, 0);

    std::cout << "Start rendering" << std::endl;
    while (!window.shouldClose()) {
        // ImGui stuff
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();

        // FPS counter + delta time calculation
        frameCounter++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - delatTimer).count();
        fpsDelta += deltaTime;
        delatTimer = currentTime;
        if (fpsDelta >= 1.0f) {
            fpsDelta -= 1.0f;
            std::cout << "FPS: " << frameCounter << std::endl;
            frameCounter = 0;
        }

        // Update
        // Get window events
        window.poolEvents();
        // Close window with ESC and Ctrl+Q
        if (window.isKeyDown(GLFW_KEY_ESCAPE) ||
            (window.isKeyDown(GLFW_KEY_Q) && window.isKeyDown(GLFW_KEY_LEFT_CONTROL)))
            window.close();
        // Camera control
        glm::vec3 deltaCamera(0);
        float speed = 0.01f;
        if (window.isKeyDown(GLFW_KEY_LEFT_CONTROL))
            speed = 0.05f;
        if (window.isKeyDown(GLFW_KEY_W))
            deltaCamera += +speed * target;
        if (window.isKeyDown(GLFW_KEY_S))
            deltaCamera += -speed * target;
        if (window.isKeyDown(GLFW_KEY_A))
            deltaCamera += -speed * glm::normalize(glm::cross(target, cameraUp));
        if (window.isKeyDown(GLFW_KEY_D))
            deltaCamera += +speed * glm::normalize(glm::cross(target, cameraUp));
        if (window.isKeyDown(GLFW_KEY_E))
            deltaCamera += -speed * cameraUp;
        if (window.isKeyDown(GLFW_KEY_Q))
            deltaCamera += +speed * cameraUp;
        origin += deltaCamera;

        if (window.isKeyDown(GLFW_KEY_C)) {
            origin = glm::vec3(0);
            target = glm::vec3(0, 0, -1);
        }
        if (window.isMouseButtonDown(GLFW_MOUSE_BUTTON_MIDDLE)) {
            MousePos d = window.getDeltaMouse();
            target = glm::rotate(glm::mat4(1.0), 0.1f * glm::radians((float) d.x), glm::vec3(0, 1, 0)) *
                     glm::vec4(target, 0);
            target = glm::rotate(glm::mat4(1.0), 0.1f * glm::radians((float) d.y), glm::vec3(1, 0, 0)) *
                     glm::vec4(target, 0);
        }

        // Update objects
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        modelMat3 = glm::scale(modelMat3Pos, glm::vec3(0.25f, 0.25f, 0.25f));
        modelMat3 = glm::rotate(modelMat3, glm::radians(30.0f),
                                glm::vec3(1.0f, 0.0f, 0.0f));
        modelMat3 = glm::rotate(modelMat3, time * glm::radians(90.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f));
        renderer.setModelMatrix(robj3, modelMat3);

        // Update camera
        renderer.setViewMatrix(glm::lookAt(origin, origin + target, cameraUp));

        // TODO: Render Entities using the RendererAPI

        // Render the frame
        renderer.drawFrame();
    }
    // Wait for renderer to finish before ending
    renderer.waitIdle();

}

// TODO: this is just a dummy for api development
struct RenderComponent {
    glm::mat4 modelMat; // Would be in Transform Component
    RendererAPI::ShaderRef shader;
    RendererAPI::MeshRef mesh;
    RendererAPI::MaterialRef material;
};

std::vector<RenderComponent> setupRendering(RendererAPI &renderer) {
    std::vector<RenderComponent> components;

    RenderComponent comp{};
    comp.modelMat = glm::identity<glm::mat4>();
    comp.shader = renderer.loadShader();
    comp.mesh = renderer.loadMesh();
    comp.material = renderer.loadMaterial();

    components.emplace_back(comp);
    return std::move(components);
}

int render(RendererAPI &renderer, const std::vector<RenderComponent> &components) {
    renderer.beginScene(/*Set Camera*/);

    for (const auto &comp : components) {
        renderer.useShader(comp.shader);
        renderer.renderObject(comp.mesh, comp.material, comp.modelMat);
    }

    renderer.endScene(/*Post Processing config*/);

    // ImGui Code
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();

    renderer.flush();
    return 0;
}


int main() {
    std::cout << "Renderer starting..." << std::endl;

    try {
        ModelLoader modelLoader;
        Window window = Window::Create("Test Engine");

        TestRenderer renderer(window);
        renderer.init();
        try {
            run(window, renderer, modelLoader);
        } catch (const std::exception &e) {
            std::cerr << "ERROR in run(): " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        try {
            modelLoader.cleanup();
            renderer.cleanup();
            window.cleanup();
        } catch (const std::exception &e) {
            std::cerr << "ERROR in cleanup: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }
    catch (const std::exception &e) {
        std::cerr << "ERROR in init: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

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
#include <src/renderer/VulkanRenderer2D.h>


#include "renderer/window/Window.h"
#include "renderer/vulkan/test/TestRenderer.h"
#include "renderer/data/Mesh.h"
#include "renderer/data/RenderObject.h"
#include "renderer/data/ModelLoader.h"

/* TODOs:
 *
	Current:
        - Proper logging
        TODO: Cleanup Builder classes

		- Learn about image layouts
		- layout transitions
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
		- Migrate to Vulkan C++ headers
        - use precompiled header
		- Memory management
			- https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
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
            - sub rendering dependencies
		- Pipeline handling
			- Shaders
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
		- TesselationControl shader
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

        // TODO: Render Entities using the RenderAPI

        // Render the frame
        renderer.drawFrame();
    }
    // Wait for renderer to finish before ending
    renderer.waitIdle();

}


void run2(Window &window, VulkanRenderer2D &renderer) {

    // FPS counter
    auto deltaTimer = std::chrono::high_resolution_clock::now();
    uint32_t frameCounter = 0;
    float fpsDelta = 0;
    // Camera
    Camera camera{
            .view = glm::mat4(1),
            .fieldOfView = 10.0f,
            .near = 0.1f,
            .far = 100.0f,
    };
    glm::vec3 origin = glm::vec3(0, 0, -2.0f);
    float cameraSpeed = 10.0f;
    bool cameraControllerActive = false;

    // Renderer configuration
    renderer.updatePostProcessingConfiguration({camera});

    std::cout << "Start rendering" << std::endl;
    while (!window.shouldClose()) {
        // ImGui stuff
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (cameraControllerActive) {
            if (ImGui::Begin("CameraControl", &cameraControllerActive)) {
                ImGui::Text("Camera Controller");
                ImGui::SliderFloat("float", &cameraSpeed, 0.0f, 50.0f);
            }
            ImGui::End();
        }
        ImGui::Render();

        // FPS counter + delta time calculation
        frameCounter++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - deltaTimer).count();
        fpsDelta += deltaTime;
        deltaTimer = currentTime;
        if (fpsDelta >= 1.0f) {
            fpsDelta -= 1.0f;
            std::cout << "FPS: " << frameCounter << std::endl;
            frameCounter = 0;
        }

        // Update
        // Get window events
        window.poolEvents();
        // TODO: Refactor Input event handling
        // Close window controls
        if (window.isKeyDown(GLFW_KEY_ESCAPE) ||
            (window.isKeyDown(GLFW_KEY_Q) && window.isKeyDown(GLFW_KEY_LEFT_CONTROL))) { window.close(); }
        if (window.isKeyDown(GLFW_KEY_LEFT_ALT) && window.isKeyDown(GLFW_KEY_LEFT_SHIFT) &&
            window.isKeyDown(GLFW_KEY_C)) { cameraControllerActive = true; }

        // Camera controls
        if (window.isKeyDown(GLFW_KEY_W)) { origin.y -= cameraSpeed * deltaTime; }
        if (window.isKeyDown(GLFW_KEY_S)) { origin.y += cameraSpeed * deltaTime; }
        if (window.isKeyDown(GLFW_KEY_A)) { origin.x += cameraSpeed * deltaTime; }
        if (window.isKeyDown(GLFW_KEY_D)) { origin.x -= cameraSpeed * deltaTime; }
        if (window.isKeyDown(GLFW_KEY_KP_ADD)) {
            camera.fieldOfView -= 5 * deltaTime;
        } // TODO: Remove delta time after input refactor
        if (window.isKeyDown(GLFW_KEY_KP_SUBTRACT)) {
            camera.fieldOfView += 5 * deltaTime;
        } // TODO: Remove delta time after input refactor

        camera.view = glm::translate(glm::mat4(1), origin);
        renderer.beginScene(camera);

        auto modelMat1 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.0f));
        renderer.renderQuad(modelMat1, glm::vec4(1.0f, 0.5f, 0.0f, 1));
        auto modelMat2 = glm::translate(glm::mat4(1.0f), glm::vec3(2, 0, 0.0f));
        renderer.renderQuad(modelMat2, glm::vec4(0.0f, 1.0f, 0.0f, 1));

        renderer.endScene();
        renderer.flush();
    }
    // Wait for renderer to finish before ending
    renderer.join();

}

//#define TESTING
#define RENDER_2D

int main() {
    std::cout << "Renderer starting..." << std::endl;

    ModelLoader modelLoader;
    Window window = Window::Create("Test Engine");

#ifdef TESTING
    TestRenderer renderer(window);
    renderer.init();
    run(window, renderer, modelLoader);
#endif
#ifdef RENDER_2D
    VulkanRenderer2D renderer2D = VulkanRenderer2D::Create(window);
    renderer2D.setup();
    run2(window, renderer2D);
#endif
    modelLoader.cleanup();

#ifdef TESTING
    renderer.cleanup();
#endif
    return EXIT_SUCCESS;
}
#include "Engine/src/ChaosEngine.h"
#include "test/TestScene.h"

#include <exception>
#include <iostream>

/* TODOs:
 *
    To Think:
		- use factory Create() instead of init (https://abseil.io/tips/42)
        - Consider PIMPL pattern https://oliora.github.io/2015/12/29/pimpl-and-rule-of-zero.html
		- Learn about image layouts +  layout transitions

	Refactor:
		- Pipeline handling
			- Shaders
        - use precompiled header
		- Migrate to Vulkan C++ headers
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
		- Swap chain
			- Cleanup
			- Creation -> Parameters

	Features:
		General:
        - Proper logging
		- Resource management
			- Model loading
			- Texture loading
			- add tinyobjloader library
                x model loading
                - material loading
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


int main() {
    std::cout << "Engine starting..." << std::endl;

    auto testScene = std::make_unique<TestScene>();
    Engine engine(std::move(testScene));
    engine.run();

    return EXIT_SUCCESS;
}
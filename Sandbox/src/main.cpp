#include "Engine/src/ChaosEngine.h"
#include "test/TestScene.h"

#include <exception>
#include <iostream>

/* TODOs:
 *
 * TODO(Current): - Change Material at Runtime
 *       - Delete Entity at Runtime
 *       - Create Entity at Runtime

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
        - Textures
		- Swap chain
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
		- Transparent Pass (Alpha blending)
		- Text rendering
		- Batch rendering
		- Lighting
		- Sky box
		- Geometry shader
		- TesselationControl shader
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
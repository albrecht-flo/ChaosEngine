#include "Engine/src/ChaosEngine.h"
//#include "test/TestScene.h"
//#include "test/EmptyScene.h"
#include "editor/EditorScene.h"

#include <nfd.h>

#include <exception>
#include <iostream>

/* TODOs:

    To Think:
		- use factory Create() instead of init (https://abseil.io/tips/42)
        - Consider PIMPL pattern https://oliora.github.io/2015/12/29/pimpl-and-rule-of-zero.html
		- Learn about image layouts +  layout transitions

	Refactor:
		- Pipeline handling
			- Shaders
        - use precompiled header
		- Migrate to Vulkan C++ headers

		- Swap chain
			- Creation -> Parameters

	Features:
		General:
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
    try {
        Logger::Init(LogLevel::Debug);
        Logger::I("Main", "Engine starting...");

//      auto scene = std::make_unique<TestScene>();
//      auto scene = std::make_unique<EmptyScene>();
        ChaosEngine::Engine engine{};
        auto scene = std::make_unique<Editor::EditorScene>();
        engine.loadScene(std::move(scene));

        NFD_Init();
        engine.run();
        NFD_Quit();
    } catch (const std::exception& ex) {
        std::cerr << "[FATAL] Unhandled exception!" << std::endl;
        std::cerr << ex.what() << std::endl;
        std::cerr << "[FATAL] Aborting!" << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "[FATAL] Unhandled AND Unknown exception!" << std::endl;
        std::cerr << "[FATAL] Aborting!" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
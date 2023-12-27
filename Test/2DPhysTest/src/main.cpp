#include "Engine/src/ChaosEngine.h"
#include "Box2DTestScene.h"

#include <exception>
#include <iostream>

int main() {
    try {
        Logger::Init(LogLevel::Debug);
        Logger::I("Main", "Engine starting...");

        ChaosEngine::Engine engine{};

        auto scene = std::make_unique<Box2DTestScene>();
        engine.loadScene(std::move(scene));

        engine.run();
    } catch (const std::exception &ex) {
        std::cerr << "[FATAL] Unhandled exception!" << std::endl;
        std::cerr << ex.what() << std::endl;
        std::cerr << "[FATAL] Aborting!" << std::endl;
        std::cin.get();
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "[FATAL] Unhandled AND Unknown exception!" << std::endl;
        std::cerr << "[FATAL] Aborting!" << std::endl;
        std::cin.get();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
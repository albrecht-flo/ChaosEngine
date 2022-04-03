#pragma once

#include "Engine/src/core/renderSystem/RenderingSystem.h"
#include "Engine/src/core/assets/Font.h"

namespace ChaosEngine {
    class UISystem {

    public:
        explicit UISystem(ChaosEngine::RenderingSystem &renderingSystem, Window &window)
                : renderingSystem(renderingSystem), window(window) {}

        void init(ECS &ecs);

        void update(ECS &ecs);

    private:
        ChaosEngine::RenderingSystem &renderingSystem;
        Window &window;
    };
}

#pragma once

#include "Engine/src/core/renderSystem/RenderingSystem.h"
#include "Font.h"

namespace ChaosEngine {
    class UISystem {

    public:
        explicit UISystem(ChaosEngine::RenderingSystem &renderingSystem) : renderingSystem(renderingSystem) {}

        void init(ECS &ecs);

        void update(ECS &ecs);

    private:
        ChaosEngine::RenderingSystem &renderingSystem;
    };
}

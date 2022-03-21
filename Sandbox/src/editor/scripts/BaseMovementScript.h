#pragma once

#include "Engine/src/core/scriptSystem/NativeScript.h"
#include "Engine/src/core/Components.h"

namespace Editor {

    class BaseMovementScript : public ChaosEngine::NativeScript {
    public:
        explicit BaseMovementScript(ChaosEngine::Entity entity) : ChaosEngine::NativeScript(entity) {}

        ~BaseMovementScript() override = default;

        void onStart() override;

        void onUpdate(float deltaTime) override;

    private:
        glm::vec3 origin{};
        float speed = 10.0f;
    };

}

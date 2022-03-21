#pragma once

#include "Engine/src/core/scriptSystem/NativeScript.h"
#include "Engine/src/core/Components.h"

namespace Editor {

    class EditorCameraScript : public ChaosEngine::NativeScript {
    public:
        explicit EditorCameraScript(ChaosEngine::Entity entity) : ChaosEngine::NativeScript(entity) {}

        ~EditorCameraScript() override = default;

        void onStart() override;

        void onUpdate(float deltaTime) override;

        void setActive(bool b);

        void setSpeed(float cameraSpeed);

    private:
        glm::vec3 origin{};
        float speed = 10.0f;
        bool active = false;
    };

}

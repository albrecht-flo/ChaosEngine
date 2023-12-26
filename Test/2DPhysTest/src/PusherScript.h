#pragma once

#include "Engine/src/core/scriptSystem/NativeScript.h"

class PusherScript : public ChaosEngine::NativeScript {
public:
    explicit PusherScript(ChaosEngine::Entity entity) : NativeScript(entity) {
    }

    ~PusherScript() override = default;

    void onStart() override;

    void onUpdate(float deltaTime) override;

private:
    glm::vec2 origin;
    float speed = 2.0f;
    float angle = -30;

    glm::vec2 path{0};
};


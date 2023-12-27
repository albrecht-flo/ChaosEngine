#pragma once

#include "Engine/src/core/scriptSystem/NativeScript.h"

class CameraScript : public ChaosEngine::NativeScript {
public:
    explicit CameraScript(ChaosEngine::Entity entity) : ChaosEngine::NativeScript(entity) {}

    ~CameraScript() override = default;

    void onStart() override;

    void onUpdate(float deltaTime) override;

    void setActive(bool b);

    void setSpeed(float cameraSpeed);

private:
    glm::vec3 origin{};
    float speed = 10.0f;
};

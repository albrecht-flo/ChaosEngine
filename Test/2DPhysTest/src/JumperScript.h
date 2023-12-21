#pragma once

#include "Engine/src/core/scriptSystem/NativeScript.h"

class JumperScript : public ChaosEngine::NativeScript {
public:
    explicit JumperScript(ChaosEngine::Entity entity, float strength) : NativeScript(entity), strength(strength) {
    }

    ~JumperScript() override = default;

    void onStart() override;

    void onUpdate(float deltaTime) override;

private:
    float strength = 10.0f;
    bool pressed = false;
};


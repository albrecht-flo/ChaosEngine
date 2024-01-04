#pragma once

#include "ButtonScript.h"

class SoundButtonScript : public ButtonScript<true> {
public:

    SoundButtonScript(ChaosEngine::Entity entity, ChaosEngine::Entity target);

    void onClick() override;

private:
    ChaosEngine::Entity target;
};
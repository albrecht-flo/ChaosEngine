#pragma once

#include "Engine/src/core/Scene.h"

#define AL_LIBTYPE_STATIC
#include <AL/al.h>

namespace ChaosEngine {

    class AudioSystem {
    public:
        AudioSystem();

        ~AudioSystem() = default;

        void init(Scene &scene);

        void update(ECS &ecs, float deltaTime);

    private:
    };

}

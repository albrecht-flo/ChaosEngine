#pragma once

#include "Engine/src/core/Scene.h"

#include "Engine/src/core/assets/RawAudio.h"

#define AL_LIBTYPE_STATIC
#include <AL/al.h>
#include <AL/alc.h>


namespace ChaosEngine {

    class AudioSystem {
    public:
        AudioSystem();

        ~AudioSystem();

        void init(Scene &scene);

        void update(ECS &ecs, float deltaTime);

    private:
        std::vector<std::string> availableAudioDevices;
        ALCdevice* openALDevice = nullptr;
        // to be moved
        ALCcontext *openALContext = nullptr;
    };

}

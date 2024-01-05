#include "AudioSystem.h"

#include "Engine/src/core/utils/Logger.h"
#include "Engine/src/core/Components.h"
#include "OpenALHelpers.h"

using namespace ChaosEngine;
using namespace OpenALHelpers;

AudioSystem::AudioSystem() {
    availableAudioDevices.clear();

    ALboolean enumeration1 = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
    ALboolean enumeration2 = alcIsExtensionPresent(NULL, "ALC_enumerate_all_EXT");
    if (enumeration1 == AL_FALSE || enumeration2 == AL_FALSE) {
        Logger::E("OpenAL", "Audio device enumeration not possible falling back to default device");
    } else {
        const ALCchar *devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
        if (reinterpret_cast<uint64_t>(devices) != AL_INVALID_ENUM) {
            const char *ptr = devices;
            while (*ptr != '\0') {
                const std::string str(ptr);
                availableAudioDevices.emplace_back(str);
                ptr += availableAudioDevices.back().size() + 1;
            }
            Logger::I("OpenAL", "Available devices");
            for (size_t i = 0; i < availableAudioDevices.size(); ++i) {
                Logger::I("OpenAL", "        Device [" + std::to_string(i) + "]: " + availableAudioDevices[i]);
            }
        } else {
            Logger::E("OpenAL", "Error while enumerating devices");
        }
    }

    if (availableAudioDevices.empty())
        openALDevice = alcOpenDevice(nullptr);
    else
        openALDevice = alcOpenDevice(availableAudioDevices[0].c_str());

    if (openALDevice == nullptr) {
        Logger::C("OpenAL", "Failed to initialize OpenAL -> AudioSystem will remain offline");
        return;
    }
    checkALCErrors(openALDevice, "TEST AFTER INIT");

    Logger::I("OpenAL", "Using audio device: " + availableAudioDevices[0]);
}

AudioSystem::~AudioSystem() {
    if (openALDevice != nullptr && openALContext != nullptr) {
        alcDestroyContext(openALContext);
        checkALCErrors(openALDevice, "Context destruction");
    }
    if (openALDevice != nullptr)
        alcCloseDevice(openALDevice);
}

void AudioSystem::init(Scene &/*scene*/) {
    if (openALDevice == nullptr)
        return;
    if (openALContext != nullptr) {
        alcDestroyContext(openALContext);
        checkALCErrors(openALDevice, "Context destruction");
    }

    openALContext = alcCreateContext(openALDevice, nullptr);
    checkALCErrors(openALDevice, "alcCreateContext");
    if (openALContext == nullptr) {
        Logger::C("OpenAL", "Failed to initialize OpenAL Context -> AudioSystem will remain offline");
        return;
    }

    if (!alcMakeContextCurrent(openALContext)) {
        checkALCErrors(openALDevice, "alcMakeContextCurrent");
        Logger::C("OpenAL", "Failed to make context current -> AudioSystem will remain offline");
        return;
    }
    checkALCErrors(openALDevice, "alcMakeContextCurrent");

    alListener3f(AL_POSITION, 0, 0, 0);
    checkALErrors("alListener3f(AL_POSITION)");
    alListener3f(AL_VELOCITY, 0, 0, 0);
    checkALErrors("alListener3f(AL_VELOCITY)");
    ALfloat orientation[] = {0, 0, -1, 0, 1, 0};
    alListenerfv(AL_ORIENTATION, orientation);
    checkALErrors("alListenerfv(AL_ORIENTATION)");
}

void AudioSystem::update(ECS &ecs, float /*deltaTime*/) {
    auto listeners = ecs.getRegistry().view<const Transform, const AudioListenerComponent>();
    auto sources = ecs.getRegistry().view<const Transform, AudioSourceComponent>();

    entt::entity mainListener = entt::null;
    for (const auto &[entity, transform, listener]: listeners.each()) {
        if (mainListener == entt::null && listener.active) {
            mainListener = entity;
            const auto &pos = transform.position;
            alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
            checkALErrors("alListener3f(AL_POSITION)");

            const auto &rotation = transform.rotation;
            auto rot = glm::quat({glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)});
            glm::vec3 up = glm::toMat3(rot) * glm::vec3{0, 1, 0};
            glm::vec3 forward = glm::toMat3(rot) * glm::vec3{0, 0, -1};
            ALfloat orientation[] = {forward.x, forward.y, forward.z, up.x, up.y, up.z};
            alListenerfv(AL_ORIENTATION, orientation);
            checkALErrors("alListenerfv(AL_ORIENTATION)");
        } else if (listener.active) {
            LOG_WARN("Only one listener can be active at a time!");
        }
    }


    for (const auto &[entity, transform, source]: sources.each()) {
        const auto &pos = transform.position;
        glm::vec3 velocity = transform.position - source.oldPosition;
        source.oldPosition = pos;
        source.source.setPositionAndVelocity(pos, velocity);
    }
}

Transform AudioSystem::GetListenerPosition() {
    float x, y, z;
    alGetListener3f(AL_POSITION, &x, &y, &z);
    checkALErrors("alGetListener3f(AL_POSITION)");
    return Transform{
            .position{x, y, z},
            .rotation{},
            .scale{}
    };
}

#include "AudioSystem.h"

#include "Engine/src/core/utils/Logger.h"

using namespace ChaosEngine;

static bool check_al_errors(const std::string& message)
{
    ALenum error = alGetError();
    if(error != AL_NO_ERROR)
    {
        Logger::E("OpenAL", "Error " + message);
        switch(error)
        {
            case AL_INVALID_NAME:
                Logger::E("OpenAL",  "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function");
                break;
            case AL_INVALID_ENUM:
                Logger::E("OpenAL",  "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function");
                break;
            case AL_INVALID_VALUE:
                Logger::E("OpenAL",  "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function");
                break;
            case AL_INVALID_OPERATION:
                Logger::E("OpenAL",  "AL_INVALID_OPERATION: the requested operation is not valid");
                break;
            case AL_OUT_OF_MEMORY:
                Logger::E("OpenAL",  "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory");
                break;
            default:
                Logger::E("OpenAL",  "UNKNOWN AL ERROR: " + std::to_string(error));
        }
        return false;
    }
    return true;
}

AudioSystem::AudioSystem() {

}

void AudioSystem::init(Scene &scene) {
    check_al_errors("Before init");
}

void AudioSystem::update(ECS &ecs, float deltaTime) {

}

#include "AudioSystem.h"

#include "Engine/src/core/utils/Logger.h"


using namespace ChaosEngine;

static bool check_al_errors(const std::string &message) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        Logger::E("OpenAL", "Error " + message);
        switch (error) {
            case AL_INVALID_NAME:
                Logger::E("OpenAL", "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function");
                break;
            case AL_INVALID_ENUM:
                Logger::E("OpenAL", "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function");
                break;
            case AL_INVALID_VALUE:
                Logger::E("OpenAL", "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function");
                break;
            case AL_INVALID_OPERATION:
                Logger::E("OpenAL", "AL_INVALID_OPERATION: the requested operation is not valid");
                break;
            case AL_OUT_OF_MEMORY:
                Logger::E("OpenAL",
                          "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory");
                break;
            default:
                Logger::E("OpenAL", "UNKNOWN AL ERROR: " + std::to_string(error));
        }
        return false;
    }
    return true;
}

static bool check_alc_errors(ALCdevice *device, const std::string &message) {
    if (device == nullptr) {
        Logger::E("OpenAL-C", "Error " + message);
        Logger::E("OpenAL-C", "alcGetError call without valid device is considered INVALID by OpenAL-Soft");
        return false;
    }
    ALCenum error = alcGetError(device);
    if (error != ALC_NO_ERROR) {
        Logger::E("OpenAL-C", "Error " + message);
        switch (error) {
            case ALC_INVALID_DEVICE:
                Logger::E("OpenAL", "ALC_INVALID_DEVICE: a bad device was passed to an OpenALC function");
                break;
            case ALC_INVALID_ENUM:
                Logger::E("OpenAL", "AL_INVALID_ENUM: an invalid enum value was passed to an OpenALC function");
                break;
            case ALC_INVALID_VALUE:
                Logger::E("OpenAL", "AL_INVALID_VALUE: an invalid value was passed to an OpenALC function");
                break;
            case ALC_INVALID_CONTEXT:
                Logger::E("OpenAL", "ALC_INVALID_CONTEXT: an invalid context was passed to an OpenALC function");
                break;
            case ALC_OUT_OF_MEMORY:
                Logger::E("OpenAL",
                          "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory");
                break;
            default:
                Logger::E("OpenAL", "UNKNOWN AL ERROR: " + std::to_string(error));
        }
        return false;
    }
    return true;
}

AudioSystem::AudioSystem() {
    ALboolean enumeration;

    enumeration = alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT");
    availableAudioDevices.clear();
    if (enumeration == AL_FALSE) {
        Logger::E("OpenAL", "Audio device enumeration not possible falling back to default device");
    } else {
        const ALCchar *defaultDevice = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
        if (defaultDevice == nullptr) {
            Logger::E("OpenAL", "Error, no audio devices present");
        } else {
            availableAudioDevices.emplace_back(defaultDevice);
        }

        const ALCchar *devices = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
        if (reinterpret_cast<uint64_t>(devices) != AL_INVALID_ENUM) {
            const char *ptr = devices;
            while (*(ptr + 1) != '\0') {
                const std::string str(ptr);
                if (std::find(availableAudioDevices.begin(),
                              availableAudioDevices.end(), str) == availableAudioDevices.end()) {
                    availableAudioDevices.emplace_back(str);
                }
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
    check_alc_errors(openALDevice, "TEST AFTER INIT");

    Logger::I("OpenAL", "Using audio device: " + availableAudioDevices[0]);
}

AudioSystem::~AudioSystem() {
    if (source1 != 0) {
        alDeleteSources(1, &source1);
        check_al_errors("alDeleteSources(source1)");
    }
    if (openALDevice != nullptr && openALContext != nullptr) {
        alcDestroyContext(openALContext);
        check_alc_errors(openALDevice, "Context destruction");
    }
    if (openALDevice != nullptr)
        alcCloseDevice(openALDevice);
}

void AudioSystem::init(Scene &scene) {
    if (openALDevice == nullptr)
        return;
    if (openALContext != nullptr) {
        alcDestroyContext(openALContext);
        check_alc_errors(openALDevice, "Context destruction");
    }

    openALContext = alcCreateContext(openALDevice, nullptr);
    check_alc_errors(openALDevice, "alcCreateContext");
    if (openALContext == nullptr) {
        Logger::C("OpenAL", "Failed to initialize OpenAL Context -> AudioSystem will remain offline");
        return;
    }

    if (!alcMakeContextCurrent(openALContext)) {
        check_alc_errors(openALDevice, "alcMakeContextCurrent");
        Logger::C("OpenAL", "Failed to make context current -> AudioSystem will remain offline");
        return;
    }
    check_alc_errors(openALDevice, "alcMakeContextCurrent");

    Transform listener{
            .position{0, 0, -2},
            .rotation{0, 0, 0},
            .scale{1},
    };
    const auto &lp = listener.position;
    alListener3f(AL_POSITION, lp.x, lp.y, lp.z);
    check_al_errors("alListener3f(AL_POSITION)");
    alListener3f(AL_VELOCITY, 0, 0, 0);
    check_al_errors("alListener3f(AL_VELOCITY)");

    const auto &rotation = listener.rotation;
    auto rot = glm::quat({glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)});
    glm::vec3 up = glm::toMat3(rot) * glm::vec3{0, 1, 0};
    glm::vec3 forward = glm::toMat3(rot) * glm::vec3{0, 0, -1};
    ALfloat orientation[] = {forward.x, forward.y, forward.z, up.x, up.y, up.z};
    alListenerfv(AL_ORIENTATION, orientation);
    check_al_errors("alListenerfv(AL_ORIENTATION)");

    alGenSources(1, &source1);
    check_al_errors("alGenSources(source1)");
    alSourcef(source1, AL_PITCH, 1);
    check_al_errors("alSourcef(AL_PITCH)");
    alSourcef(source1, AL_GAIN, 1);
    check_al_errors("alSourcef(AL_GAIN)");
    alSource3f(source1, AL_POSITION, 10, 0, -2);
    check_al_errors("alSource3f(AL_POSITION)");
    alSource3f(source1, AL_VELOCITY, 0, 0, 0);
    check_al_errors("alSource3f(AL_VELOCITY)");
    alSourcei(source1, AL_LOOPING, AL_TRUE);
    check_al_errors("alSourcei(AL_LOOPING)");

    auto audio = RawAudio::loadOggFile("sounds/test.ogg");
    ALenum format = getALFormat(audio.getFormat());

    alGenBuffers((ALuint) 1, &buffer1);
    check_al_errors("alGenSources(buffer1)");
    alBufferData(buffer1, format, audio.getData(), (int) audio.getSize(), audio.getSampleRate());
    check_al_errors("alBufferData(buffer1)");

    alSourcei(source1, AL_BUFFER, buffer1);
    check_al_errors("alSourcei(AL_BUFFER)");
    alSourcePlay(source1);
    check_al_errors("alSourcePlay(source1)");
}

void AudioSystem::update(ECS &ecs, float deltaTime) {

}

ALenum AudioSystem::getALFormat(const AudioFormat &format) {
    switch (format) {
        case AudioFormat::MONO_8:
            return AL_FORMAT_MONO8;
        case AudioFormat::MONO_16:
            return AL_FORMAT_MONO16;
        case AudioFormat::STEREO_8:
            return AL_FORMAT_STEREO8;
        case AudioFormat::STEREO_16:
            return AL_FORMAT_STEREO16;
        default:
            assert("Invalid audio format for OpenAL" && false);
    }
    return AL_FORMAT_MONO8;
}

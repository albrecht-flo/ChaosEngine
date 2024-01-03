#include "OpenALHelpers.h"

#include "Engine/src/core/utils/Logger.h"

using namespace ChaosEngine;

static void logALError(ALenum error, const std::string &message) {
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
}


static void logALCError(ALenum error, const std::string &message) {
    Logger::E("OpenAL-C", "Error " + message);
    switch (error) {
        case ALC_INVALID_DEVICE:
            Logger::E("OpenAL-C", "ALC_INVALID_DEVICE: a bad device was passed to an OpenALC function");
            break;
        case ALC_INVALID_ENUM:
            Logger::E("OpenAL-C", "AL_INVALID_ENUM: an invalid enum value was passed to an OpenALC function");
            break;
        case ALC_INVALID_VALUE:
            Logger::E("OpenAL-C", "AL_INVALID_VALUE: an invalid value was passed to an OpenALC function");
            break;
        case ALC_INVALID_CONTEXT:
            Logger::E("OpenAL-C", "ALC_INVALID_CONTEXT: an invalid context was passed to an OpenALC function");
            break;
        case ALC_OUT_OF_MEMORY:
            Logger::E("OpenAL-C",
                      "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory");
            break;
        default:
            Logger::E("OpenAL", "UNKNOWN AL ERROR: " + std::to_string(error));
    }
}

namespace ChaosEngine {
    bool OpenALHelpers::checkALErrors(const std::string &message) {
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            logALError(error, message);
            return false;
        }
        return true;
    }

    bool OpenALHelpers::checkALErrors(const std::string &message, ALuint handle) {
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            logALError(error, message + "-" + std::to_string(handle));
            return false;
        }
        return true;
    }

    bool OpenALHelpers::checkALCErrors(ALCdevice *device, const std::string &message) {
        if (device == nullptr) {
            Logger::E("OpenAL-C", "Error " + message);
            Logger::E("OpenAL-C", "alcGetError call without valid device is considered INVALID by OpenAL-Soft");
            return false;
        }

        ALCenum error = alcGetError(device);
        if (error != ALC_NO_ERROR) {
            logALCError(error, message);
            return false;
        }
        return true;
    }

    bool OpenALHelpers::checkALCErrors(ALCdevice *device, const std::string &message, ALuint handle) {
        if (device == nullptr) {
            Logger::E("OpenAL-C", "Error " + message);
            Logger::E("OpenAL-C", "alcGetError call without valid device is considered INVALID by OpenAL-Soft");
            return false;
        }

        ALCenum error = alcGetError(device);
        if (error != ALC_NO_ERROR) {
            logALCError(error, message + "-" + std::to_string(handle));
            return false;
        }
        return true;
    }

    ALenum OpenALHelpers::getALFormat(const AudioFormat &format) {
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
}
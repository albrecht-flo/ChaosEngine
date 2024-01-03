#pragma once

#include <string>

#define AL_LIBTYPE_STATIC

#include <AL/al.h>
#include <AL/alc.h>

#include "Engine/src/core/assets/RawAudio.h"

namespace ChaosEngine::OpenALHelpers {

    bool checkALErrors(const std::string &message);

    bool checkALErrors(const std::string &message, ALuint handle);

    bool checkALCErrors(ALCdevice *device, const std::string &message);

    bool checkALCErrors(ALCdevice *device, const std::string &message, ALuint handle);

    ALenum getALFormat(const AudioFormat &format);
}

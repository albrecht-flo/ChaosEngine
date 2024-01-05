#include "AudioBuffer.h"

#include <stdexcept>
#include <cassert>

#define AL_LIBTYPE_STATIC

#include <AL/al.h>

#include "Engine/src/core/audioSystem/OpenALHelpers.h"
#include "Engine/src/core/assets/RawAudio.h"

using namespace ChaosEngine;
using namespace OpenALHelpers;

std::shared_ptr<AudioBuffer> AudioBuffer::Create(const std::string &filename) {
    ALuint buffer;

    auto audio = RawAudio::loadOggFile(filename);
    ALenum format = getALFormat(audio.getFormat());

    alGenBuffers(1, &buffer);
    checkALErrors("alGenSources", buffer);

    if (!buffer)
        throw std::runtime_error("[AudioBuffer] Failed to create new OpenAL buffer");

    alBufferData(buffer, format, audio.getData(), (int) audio.getSize(), audio.getSampleRate());
    checkALErrors("alBufferData", buffer);

    return std::make_shared<AudioBuffer>(buffer, audio.getFormat(), audio.getSampleRate(), audio.getChannels(),
                                         audio.getSamples(), RawAudio(0, 0, 0, nullptr));
}

void AudioBuffer::destroy() {
    if (handle == 0)
        return;

    alDeleteBuffers(1, &handle);
    checkALErrors("alDeleteBuffers", handle);
}

uint32_t AudioBuffer::getSampleSize() const {
    switch (format) {
        case AudioFormat::MONO_8:
        case AudioFormat::STEREO_8:
            return sizeof(char);
        case AudioFormat::MONO_16:
        case AudioFormat::STEREO_16:
            return sizeof(short);
        default:
            assert("Unsupported audio format by AudioBuffer" && false);
    }
    return 0;
}

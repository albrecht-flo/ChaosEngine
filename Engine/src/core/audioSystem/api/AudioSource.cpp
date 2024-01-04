#include "AudioSource.h"

#include <stdexcept>

#define AL_LIBTYPE_STATIC

#include <AL/al.h>

#include "Engine/src/core/audioSystem/OpenALHelpers.h"

using namespace ChaosEngine;
using namespace OpenALHelpers;

AudioSource AudioSource::Create(const glm::vec3 &position, bool looping) {
    ALuint source;
    alGenSources(1, &source);
    checkALErrors("alGenSources", source);
    if (source == 0)
        throw std::runtime_error("[AudioSource] Failed to create new OpenAL source");

    alSourcef(source, AL_PITCH, 1);
    checkALErrors("alSourcef(AL_PITCH)", source);
    alSourcef(source, AL_GAIN, 1);
    checkALErrors("alSourcef(AL_GAIN)", source);
    alSource3f(source, AL_POSITION, position.x, position.y, position.z);
    checkALErrors("alSource3f(AL_POSITION)", source);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    checkALErrors("alSource3f(AL_VELOCITY)", source);
    alSourcei(source, AL_LOOPING, (looping) ? AL_TRUE : AL_FALSE);
    checkALErrors("alSourcei(AL_LOOPING)", source);

    return AudioSource{source};
}

void AudioSource::destroy() {
    if (handle == 0)
        return;

    alDeleteSources(1, &handle);
    checkALErrors("alDeleteSources", handle);
}

void AudioSource::setBuffer(std::shared_ptr<AudioBuffer> nbuffer) {
    buffer = std::move(nbuffer);

    alSourcei(handle, AL_BUFFER, (int) buffer->handle);
    checkALErrors("alSourcei(AL_BUFFER)", handle);
}

void AudioSource::play() {
    alSourcePlay(handle);
    checkALErrors("alSourcePlay", handle);
}

void AudioSource::pause() {
    alSourcePause(handle);
    checkALErrors("alSourcePause", handle);
}

void AudioSource::stop() {
    alSourceStop(handle);
    checkALErrors("alSourceStop", handle);
}

void AudioSource::setPosition(const glm::vec3 &position) {
    alSource3f(handle, AL_POSITION, position.x, position.y, position.z);
    checkALErrors("alSource3f(AL_POSITION)", handle);
}

void AudioSource::setVelocity(const glm::vec3 &velocity) {
    alSource3f(handle, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    checkALErrors("alSource3f(AL_VELOCITY)", handle);
}

void AudioSource::setPositionAndVelocity(const glm::vec3 &position, const glm::vec3 &velocity) {
    alSource3f(handle, AL_POSITION, position.x, position.y, position.z);
    checkALErrors("alSource3f(AL_POSITION)", handle);
    alSource3f(handle, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    checkALErrors("alSource3f(AL_VELOCITY)", handle);

}

void AudioSource::setPitch(float pitch) {
    alSourcef(handle, AL_PITCH, pitch);
    checkALErrors("alSourcef(AL_PITCH)", handle);
}

void AudioSource::setGain(float gain) {
    alSourcef(handle, AL_GAIN, gain);
    checkALErrors("alSourcef(AL_GAIN)", handle);
}

void AudioSource::setLooping(bool looping) {
    alSourcei(handle, AL_LOOPING, (looping) ? AL_TRUE : AL_FALSE);
    checkALErrors("alSourcei(AL_LOOPING)", handle);
}

size_t AudioSource::getBufferPosition() const {
    ALint res = 0;
    alGetSourcei(handle, AL_BYTE_OFFSET, &res);
    return res;
}

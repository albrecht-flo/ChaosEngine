#pragma once

#include <string>
#include <memory>
#include "core/assets/RawAudio.h"

namespace ChaosEngine {
    class AudioBuffer {
    public:
        explicit AudioBuffer(uint32_t handle, AudioFormat format, int sampleRate, int channels, int samples)
                : handle(handle), format(format), sampleRate(sampleRate), channels(channels), samples(samples) {}

        ~AudioBuffer() { destroy(); }

        AudioBuffer(const AudioBuffer &o) = delete;

        AudioBuffer &operator=(const AudioBuffer &o) = delete;

        AudioBuffer(AudioBuffer &&o) noexcept
                : handle(o.handle), format(o.format), sampleRate(o.sampleRate), channels(o.channels),
                  samples(o.samples) { o.handle = 0; }

        AudioBuffer &operator=(AudioBuffer &&o) noexcept {
            if (&o == this)
                return *this;
            destroy();
            handle = o.handle;
            o.handle = 0;

            format = o.format;
            sampleRate = o.sampleRate;
            channels = o.channels;
            samples = o.samples;
            return *this;
        }

        // -------------------------------------------------------------------------------------------------------------

        static std::shared_ptr<AudioBuffer> Create(const std::string &filename);

        // -------------------------------------------------------------------------------------------------------------

        [[nodiscard]] int getChannels() const { return channels; }

        [[nodiscard]] int getSampleRate() const { return sampleRate; }

        [[nodiscard]] int getSamples() const { return samples; }

        [[nodiscard]] size_t getSize() const { return sizeof(short) * samples * channels; }

        [[nodiscard]] AudioFormat getFormat() const { return format; }

        [[nodiscard]] uint32_t getSampleSize() const;

    private:
        void destroy();

    private:
        friend class AudioSource;

        uint32_t handle;
        AudioFormat format;
        int sampleRate;
        int channels;
        int samples;
    };
}


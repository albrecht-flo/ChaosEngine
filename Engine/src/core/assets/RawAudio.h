#pragma once

#include <string>
#include <memory>

namespace ChaosEngine {

    enum class AudioFormat {
        MONO_8, STEREO_8, MONO_16, STEREO_16
    };

    class RawAudio {
    public:
        RawAudio(int channels, int sampleRate, int samples, short* data);

        ~RawAudio() {
            free(data);
        };

        RawAudio(const RawAudio &o) = delete;

        RawAudio &operator=(const RawAudio &o) = delete;

        RawAudio(RawAudio &&o) noexcept
                : channels(o.channels), sampleRate(o.sampleRate), samples(o.samples), data(o.data),
                  format(o.format) {}

        RawAudio &operator=(RawAudio &&o) noexcept {
            if (&o == this)
                return *this;
            if(data != nullptr)
                free(data);
            channels = o.channels;
            sampleRate = o.sampleRate;
            samples = o.samples;
            data = o.data;
            format = o.format;
            return *this;
        }

        // Creation api --------------------------------------------------------------------------

        static RawAudio loadOggFile(const std::string &filename);

        // Getter --------------------------------------------------------------------------

        [[nodiscard]] int getChannels() const { return channels; }

        [[nodiscard]] int getSampleRate() const { return sampleRate; }

        [[nodiscard]] int getSamples() const { return samples; }

        [[nodiscard]] short *getData() const { return data; }

        [[nodiscard]] size_t getSize() const { return sizeof(short) * samples * channels; }

        [[nodiscard]] AudioFormat getFormat() const { return format; }

    private:
        int channels;
        int sampleRate;
        int samples;
        short* data;
        AudioFormat format;
    };
}
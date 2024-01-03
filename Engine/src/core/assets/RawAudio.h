#pragma once

#include <string>
#include <memory>

namespace ChaosEngine {

    enum class AudioFormat {
        MONO_8, STEREO_8, MONO_16, STEREO_16
    };

    class RawAudio {
    public:
        RawAudio(int channels, int sampleRate, int samples, std::unique_ptr<short> data);

        ~RawAudio() = default;

        RawAudio(const RawAudio &o) = delete;

        RawAudio &operator=(const RawAudio &o) = delete;

        RawAudio(RawAudio &&o) = delete;

        RawAudio &operator=(RawAudio &&o) = delete;

        // Creation api --------------------------------------------------------------------------

        static RawAudio loadOggFile(const std::string &filename);

        // Getter --------------------------------------------------------------------------

        [[nodiscard]] int getChannels() const { return channels; }

        [[nodiscard]] int getSampleRate() const { return sampleRate; }

        [[nodiscard]] int getSamples() const { return samples; }

        [[nodiscard]] short *getData() const { return data.get(); }

        [[nodiscard]] size_t getSize() const { return sizeof(short) * samples * channels; }

        [[nodiscard]] AudioFormat getFormat() const { return format; }

    private:
        int channels;
        int sampleRate;
        int samples;
        std::unique_ptr<short> data;
        AudioFormat format;
    };
}
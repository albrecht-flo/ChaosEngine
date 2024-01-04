#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "AudioBuffer.h"

namespace ChaosEngine {

    class AudioSource {
    public:
        explicit AudioSource(uint32_t handle) : handle(handle) {}

        ~AudioSource() { destroy(); }

        AudioSource(const AudioSource &o) = delete;

        AudioSource &operator=(const AudioSource &o) = delete;

        AudioSource(AudioSource &&o) noexcept
                : handle(o.handle), buffer(std::move(o.buffer)) { o.handle = 0; }

        AudioSource &operator=(AudioSource &&o) noexcept {
            if (&o == this)
                return *this;
            destroy();
            handle = o.handle;
            o.handle = 0;
            buffer = std::move(o.buffer);
            return *this;
        }

        // -------------------------------------------------------------------------------------------------------------

        static AudioSource Create(const glm::vec3 &position, bool looping = false);

        // -------------------------------------------------------------------------------------------------------------

        void setBuffer(std::shared_ptr<AudioBuffer> buffer);

        void play();

        void pause();

        void stop();

        void setPitch(float pitch);

        void setGain(float gain);

        void setLooping(bool looping);

        size_t getBufferPosition() const;

        const AudioBuffer &getBuffer() const { return *buffer; }

    private:
        void destroy();

    private:
        friend class AudioSystem;

        void setPosition(const glm::vec3 &position);

        void setVelocity(const glm::vec3 &velocity);

        void setPositionAndVelocity(const glm::vec3 &position, const glm::vec3 &velocity);

    private:
        uint32_t handle;
        std::shared_ptr<AudioBuffer> buffer = nullptr;
    };

}

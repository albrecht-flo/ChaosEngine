#include "RawAudio.h"

#include <filesystem>

#include "Engine/src/core/utils/Logger.h"

#define STB_VORBIS_HEADER_ONLY

#include "stb_vorbis.c"

using namespace ChaosEngine;

static void show_info(stb_vorbis_info &info, const std::string &filename) {
    LOG_DEBUG("[RawAudio] ({}) {} channels, {} samples/sec", filename, info.channels, info.sample_rate);
    LOG_DEBUG("[RawAudio] ({}) Predicted memory needed: {} ({} + {})", filename,
              info.setup_memory_required + info.temp_memory_required,
              info.setup_memory_required, info.temp_memory_required);
}

RawAudio RawAudio::loadOggFile(const std::string &filename) {

    if (!std::filesystem::exists(filename)) {
        throw std::runtime_error("[stb_vorbis] File does not exist " + filename);
    }

//    int vorbisErr = 0;
//    std::unique_ptr<stb_vorbis, void (*)(stb_vorbis *ptr)>
//            oggStream(stb_vorbis_open_filename(filename.c_str(), &vorbisErr, nullptr),
//                      [](stb_vorbis *ptr) -> void {
//                          stb_vorbis_close(ptr);
//                      });
//
//    if (oggStream == nullptr) {
//        throw std::runtime_error("[stb_vorbis] Failed to open audio file " + filename);
//    }
//    stb_vorbis_info info = stb_vorbis_get_info(oggStream.get());
//    show_info(info, filename);

    short *output = nullptr;
    int channels, sample_rate;
    int samples = stb_vorbis_decode_filename(filename.c_str(), &channels, &sample_rate, &output);
    if (samples == -1) {
        throw std::runtime_error("[stb_vorbis] Failed to decode audio file " + filename);
    }

    return RawAudio{channels, sample_rate, samples, output};
}

RawAudio::RawAudio(int channels, int sampleRate, int samples, short *data)
        : channels(channels), sampleRate(sampleRate), samples(samples), data(data),
          format(AudioFormat::MONO_8) {
    if (channels == 1) {
        format = AudioFormat::MONO_16;
    } else {
        format = AudioFormat::STEREO_16;
    }
    LOG_INFO("[RawAudio] RawAudio({}, {}, {}, ptr, {})", channels, sampleRate, samples, format);
}

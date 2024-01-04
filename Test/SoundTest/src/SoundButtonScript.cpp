#include "SoundButtonScript.h"

void SoundButtonScript::onClick() {
    if (!target.has<AudioSourceComponent>()) {
        LOG_WARN("SoundButtonScript requires an audio source component on its target");
        return;
    }
    target.get<AudioSourceComponent>().source.play();
}

SoundButtonScript::SoundButtonScript(ChaosEngine::Entity entity, ChaosEngine::Entity target)
        : ButtonScript(entity), target(target) {
    if (!target.has<AudioSourceComponent>())
        LOG_WARN("SoundButtonScript requires an audio source component on its target");
}

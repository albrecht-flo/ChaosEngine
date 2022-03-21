#include "NativeScriptSystem.h"
#include "Engine/src/core/Ecs.h"
#include "Engine/src/core/Components.h"

using namespace ChaosEngine;

void NativeScriptSystem::init(ChaosEngine::ECS &ecs) {
    auto scripts = ecs.getRegistry().view<NativeScriptComponent>();

    for (auto&&[entity, scriptComponent]: scripts.each()) {
        if (scriptComponent.script == nullptr || !scriptComponent.active)
            continue;
        if (!scriptComponent.initialized) {
            scriptComponent.script->onStart();
            scriptComponent.initialized = true;
        }
    }

}

void NativeScriptSystem::update(ChaosEngine::ECS &ecs, float deltaTime) {
    auto scripts = ecs.getRegistry().view<NativeScriptComponent>();

    for (auto&&[entity, scriptComponent]: scripts.each()) {
        if (scriptComponent.script == nullptr || !scriptComponent.active)
            continue;
        if (!scriptComponent.initialized) {
            scriptComponent.script->onStart();
            scriptComponent.initialized = true;
        }
        scriptComponent.script->onUpdate(deltaTime);
    }

}

#include "NativeScript.h"

#include "Engine/src/core/Engine.h"

bool ChaosEngine::NativeScript::isKeyDown(int keyCode) {
    return Engine::getEngineInstance()->getEngineWindow().isKeyDown(keyCode);
}

bool ChaosEngine::NativeScript::isKeyUp(int keyCode) {
    return Engine::getEngineInstance()->getEngineWindow().isKeyUp(keyCode);
}

bool ChaosEngine::NativeScript::isMouseButtonDown(int button) {
    return Engine::getEngineInstance()->getEngineWindow().isMouseButtonDown(button);
}

bool ChaosEngine::NativeScript::isMouseButtonUp(int button) {
    return Engine::getEngineInstance()->getEngineWindow().isMouseButtonUp(button);
}

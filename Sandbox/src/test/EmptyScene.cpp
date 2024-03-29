#include "EmptyScene.h"

ChaosEngine::SceneConfiguration EmptyScene::configure(ChaosEngine::Engine &engine) {
    window = &engine.getEngineWindow();
    return ChaosEngine::SceneConfiguration{
            .rendererType = Renderer::RendererType::RENDERER2D,
            .renderSceneToOffscreenBuffer = false,
    };
}


void EmptyScene::load() {

}

void EmptyScene::update(float /*deltaTime*/) {

}

void EmptyScene::updateImGui() {

}
#include "EmptyScene.h"

ChaosEngine::SceneConfiguration EmptyScene::configure(Window &pWindow) {
    window = &pWindow;
    return ChaosEngine::SceneConfiguration{
            .rendererType = Renderer::RendererType::RENDERER2D
    };
}


void EmptyScene::load() {

}

void EmptyScene::update(float /*deltaTime*/) {

}

void EmptyScene::updateImGui() {

}
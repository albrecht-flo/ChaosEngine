#include "EmptyScene.h"

SceneConfiguration EmptyScene::configure(Window &pWindow) {
    window = &pWindow;
    return SceneConfiguration{
            .rendererType = Renderer::RendererType::RENDERER2D
    };
}


void EmptyScene::load() {

}

void EmptyScene::update(float /*deltaTime*/) {

}

void EmptyScene::updateImGui() {

}
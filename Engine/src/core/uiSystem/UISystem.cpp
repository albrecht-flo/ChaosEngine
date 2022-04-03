#include "UISystem.h"

#include "core/utils/Logger.h"
#include "Engine/src/core/Components.h"
#include "Engine/src/renderer/window/Window.h"

using namespace ChaosEngine;

void UISystem::init(ECS &ecs) {

}

static int isLeft(glm::vec2 p1, glm::vec2 p2, glm::vec2 p) {
    return (p2.x - p1.x) * (p.y - p1.y) - (p.x - p1.x) * (p2.y - p1.y) > 0;
}

static bool pointInRectangle(const Transform &transform, glm::vec2 mousePosition) {
    const auto &p = transform.position;
    const auto &s = transform.scale;
    glm::vec2 lb{p.x - s.x, p.y - s.y};
    glm::vec2 lt{p.x - s.x, p.y + s.y};
    glm::vec2 rt{p.x + s.x, p.y + s.y};
    glm::vec2 rb{p.x + s.x, p.y - s.y};
    return !isLeft(lb, lt, mousePosition) && !isLeft(lt, rt, mousePosition) &&
           !isLeft(rt, rb, mousePosition) && !isLeft(rb, lb, mousePosition);
}

void UISystem::update(ECS &ecs) {
    auto scripts = ecs.getRegistry().view<Transform, UIComponent>();

    // Handle mouse input and dispatch events for UI Component
    auto mouse = window.getAbsoluteMousePos();
    auto viewportExtent = window.getGameWindowExtent();
    // Check if mouse is in scene viewport
    if (mouse.x < viewportExtent.first.x || mouse.y < viewportExtent.first.y || mouse.x > viewportExtent.second.x ||
        mouse.y > viewportExtent.second.y)
        return;

    // Relative mouse position in viewport
    glm::vec2 mousePos{mouse.x - viewportExtent.first.x, mouse.y - viewportExtent.first.y};
//    LOG_DEBUG("Mouse position {}x{}", mousePos.x, mousePos.y);
    for (auto&&[entity, transform, ui]: scripts.each()) {
        if (!ui.clickable)
            continue;

        if (pointInRectangle(transform, mousePos)) {
            Entity entityH = ecs.getEntity(entity);
            if (entityH.has<NativeScriptComponent>()) {
                auto &script = entityH.get<NativeScriptComponent>();
                if (script.active) {
                    script.script->onMouseOver();
                }
            }
        }
    }

}

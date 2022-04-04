#include "UISystem.h"

#include <glm/gtx/quaternion.hpp>
#include "core/utils/Logger.h"
#include "Engine/src/core/Components.h"
#include "Engine/src/renderer/window/Window.h"

using namespace ChaosEngine;

// ------------------------------------ Helper functions ---------------------------------------------------------------

static int isLeft(glm::vec2 p1, glm::vec2 p2, glm::vec2 p) {
    return (p2.x - p1.x) * (p.y - p1.y) - (p.x - p1.x) * (p2.y - p1.y) > 0;
}

static bool pointInRectangle(const Transform &transform, glm::vec2 mousePosition) {
    bool flipX = transform.rotation.x != 0 && transform.rotation.y != 0;
    glm::mat4 modelMat = glm::translate(glm::mat4(1), transform.position);
    // the rotation around the z and x-axis needs to be flipped due to the projection
    modelMat *= glm::toMat4(glm::quat(glm::vec3{(flipX ? -1.0f : 1) * glm::radians(transform.rotation.x),
                                                glm::radians(transform.rotation.y),
                                                -glm::radians(transform.rotation.z)}));
    modelMat = glm::scale(modelMat, transform.scale);

    glm::vec2 lb = glm::vec2(modelMat * glm::vec4{-1.0f, -1.0f, 0.0f, 1.0f});
    glm::vec2 lt = glm::vec2(modelMat * glm::vec4{-1.0f, +1.0f, 0.0f, 1.0f});
    glm::vec2 rt = glm::vec2(modelMat * glm::vec4{+1.0f, +1.0f, 0.0f, 1.0f});
    glm::vec2 rb = glm::vec2(modelMat * glm::vec4{+1.0f, -1.0f, 0.0f, 1.0f});
    return !isLeft(lb, lt, mousePosition) && !isLeft(lt, rt, mousePosition) &&
           !isLeft(rt, rb, mousePosition) && !isLeft(rb, lb, mousePosition);;
}

static bool pointInAxisAlignedBox(const Transform &transform, glm::vec2 mousePosition) {
    const auto &p = transform.position;
    const auto &s = transform.scale;
    return (p.x - s.x <= mousePosition.x && mousePosition.x <= p.x + s.x) &&
           (p.y - s.y <= mousePosition.y && mousePosition.y <= p.y + s.y);
}

// ------------------------------------ Class Members ------------------------------------------------------------------

void UISystem::init(ECS &ecs) {

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
    // LOG_DEBUG("Mouse position {}x{}", mousePos.x, mousePos.y);
    for (auto&&[entity, transform, ui]: scripts.each()) {
        if (!ui.active)
            continue;
        bool isMouseOver = (transform.rotation == glm::vec3(0, 0, 0)) ? pointInAxisAlignedBox(transform, mousePos)
                                                                      : pointInRectangle(transform, mousePos);
        if (isMouseOver) {
            Entity entityH = ecs.getEntity(entity);
            if (entityH.has<NativeScriptComponent>()) {
                auto &script = entityH.get<NativeScriptComponent>();
                if (script.active) {
                    // LOG_DEBUG("Collision {:x}", entity);
                    script.script->onMouseOver();
                }
            }
        }
    }

}

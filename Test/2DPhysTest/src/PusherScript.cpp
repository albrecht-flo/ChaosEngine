#include "PusherScript.h"
#include "Engine/src/core/Components.h"
#include "Engine/src/core/utils/GLMCustomExtension.h"

using namespace GLMCustomExtension;

void PusherScript::onStart() {
    origin = glm::vec2{getComponent<TransformComponent>().local.position};
    auto anchor = origin + glm::normalize(rotation2D(angle) * glm::vec2(1, 0));
    path = origin - anchor;

    auto &body = getComponent<DynamicRigidBodyComponent>().body;
    auto velocity = glm::normalize(rotation2D(angle) * glm::vec2(1, 0)) * speed;
    body.setVelocity(glm::vec3(velocity, 0));
}

void PusherScript::onUpdate(float) {
    auto curP = glm::vec2{getComponent<TransformComponent>().local.position};

    float distance = glm::distance(origin, curP);
    if (glm::dot(path, origin - curP) < 0)
        distance = -distance;
    if ((distance > 6.0f && speed > 0) || (distance < -1.0f && speed < 0)) {
        speed = -speed;

        auto &body = getComponent<DynamicRigidBodyComponent>().body;
        auto velocity = glm::normalize(rotation2D(angle) * glm::vec2(1, 0)) * speed;
        body.setVelocity(glm::vec3(velocity, 0));
    }
}

#include "JumperScript.h"
#include "Engine/src/core/Components.h"
#include "GLFW/glfw3.h"

void JumperScript::onStart() {
}

void JumperScript::onUpdate(float /*deltaTime*/) {
    if (isKeyDown(GLFW_KEY_SPACE) && !pressed) {
        getComponent<DynamicRigidBodyComponent>().body.setVelocity(glm::vec3(0, strength, 0));
        pressed = true;
        LOG_DEBUG("Jumping :)");
    }
    if (isKeyUp(GLFW_KEY_SPACE) && pressed)
        pressed = false;
}

void JumperScript::onCollisionEnter(const ChaosEngine::Entity& /*other*/) {
    LOG_DEBUG("Jumper script hit collision Enter :D");
}

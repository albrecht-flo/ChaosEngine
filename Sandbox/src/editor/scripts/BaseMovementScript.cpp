#include "BaseMovementScript.h"
#include "GLFW/glfw3.h"

using namespace Editor;

void BaseMovementScript::onStart() {
    transform = &getComponent<Transform>();
    origin = transform->position;
}


void BaseMovementScript::onUpdate(float deltaTime) {
    if (isKeyDown(GLFW_KEY_UP)) { origin.y += speed * deltaTime; }
    if (isKeyDown(GLFW_KEY_DOWN)) { origin.y -= speed * deltaTime; }
    if (isKeyDown(GLFW_KEY_LEFT)) { origin.x -= speed * deltaTime; }
    if (isKeyDown(GLFW_KEY_RIGHT)) { origin.x += speed * deltaTime; }
    transform->position = origin;
}

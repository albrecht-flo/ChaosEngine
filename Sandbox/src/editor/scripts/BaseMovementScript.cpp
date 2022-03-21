#include "BaseMovementScript.h"
#include "GLFW/glfw3.h"

using namespace Editor;

void BaseMovementScript::onStart() {
    origin = getComponent<Transform>().position;
}


void BaseMovementScript::onUpdate(float deltaTime) {
    if (isKeyDown(GLFW_KEY_UP)) { origin.y += speed * deltaTime; }
    if (isKeyDown(GLFW_KEY_DOWN)) { origin.y -= speed * deltaTime; }
    if (isKeyDown(GLFW_KEY_LEFT)) { origin.x -= speed * deltaTime; }
    if (isKeyDown(GLFW_KEY_RIGHT)) { origin.x += speed * deltaTime; }
    getComponent<Transform>().position = origin;
}

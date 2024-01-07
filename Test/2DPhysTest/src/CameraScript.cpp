#include "CameraScript.h"
#include "Engine/src/core/Components.h"
#include "GLFW/glfw3.h"


void CameraScript::onStart() {
    origin = getComponent<TransformComponent>().local.position;
}


void CameraScript::onUpdate(float deltaTime) {
    if (isKeyDown(GLFW_KEY_W)) { origin.y -= speed * deltaTime; }
    if (isKeyDown(GLFW_KEY_S)) { origin.y += speed * deltaTime; }
    if (isKeyDown(GLFW_KEY_A)) { origin.x += speed * deltaTime; }
    if (isKeyDown(GLFW_KEY_D)) { origin.x -= speed * deltaTime; }
    if (isKeyDown(GLFW_KEY_KP_ADD)) {
        getComponent<CameraComponent>().fieldOfView -= 5 * deltaTime;
    }
    if (isKeyDown(GLFW_KEY_KP_SUBTRACT)) {
        getComponent<CameraComponent>().fieldOfView += 5 * deltaTime;
    }

    getComponent<TransformComponent>().local.position = origin;
}

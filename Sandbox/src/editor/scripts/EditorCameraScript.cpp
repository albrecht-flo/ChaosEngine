#include "EditorCameraScript.h"
#include "GLFW/glfw3.h"

using namespace Editor;

void EditorCameraScript::onStart() {
    origin = getComponent<Transform>().position;
}


void EditorCameraScript::onUpdate(float deltaTime) {
    if (!active)
        return;

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

    getComponent<Transform>().position = origin;
}

void EditorCameraScript::setActive(bool b) {
    active = b;
}

void EditorCameraScript::setSpeed(float cameraSpeed) {
    speed = cameraSpeed;
}

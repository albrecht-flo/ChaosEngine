#pragma once

#include "Engine/src/core/scriptSystem/NativeScript.h"
#include <GLFW/glfw3.h>
#include "Engine/src/core/Components.h"

namespace Editor {

    template<bool DEBUG>
    class ButtonScript : public ChaosEngine::NativeScript {
    public:
        explicit ButtonScript(ChaosEngine::Entity entity) : ChaosEngine::NativeScript(entity) {}

        ~ButtonScript() override = default;

        void onStart() override {}

        void onUpdate(float /*deltaTime*/) override {
            if (mouseOver) {
                if (!mouseEntered) {
                    onMouseEnter();
                    mouseEntered = true;
                }
                if (isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) && !clicked) {
                    clicked = true;
                }
                if (isMouseButtonUp(GLFW_MOUSE_BUTTON_LEFT) && clicked) {
                    onClick();
                    clicked = false;
                }
            } else {
                if (mouseEntered) {
                    onMouseExit();
                    mouseEntered = false;
                }
                clicked = false;
            }

            mouseOver = false;
        }

        void onMouseOver() override {
            mouseOver = true;
        }

        // ------------------------------------ Button Events ----------------------------------------------------------

        virtual void onClick() {
            if (DEBUG)
                LOG_DEBUG("[ButtonBlueprintScript] onClick {}",
                          hasComponent<Meta>() ? getComponent<Meta>().name.c_str() : "No name");
        }

        virtual void onMouseEnter() {
            if (DEBUG)
                LOG_DEBUG("[ButtonBlueprintScript] mouseEnter {}",
                          hasComponent<Meta>() ? getComponent<Meta>().name.c_str() : "No name");
        }

        virtual void onMouseExit() {
            if (DEBUG)
                LOG_DEBUG("[ButtonBlueprintScript] mouseExit {}",
                          hasComponent<Meta>() ? getComponent<Meta>().name.c_str() : "No name");
        }

    private:
        bool mouseOver = false;
        bool clicked = false;
        bool mouseEntered = false;
    };

}

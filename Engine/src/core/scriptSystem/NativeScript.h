#pragma once

#include "Engine/src/core/Entity.h"
#include "Engine/src/core/utils/Logger.h"
#include <glm/glm.hpp>

class Window;

namespace ChaosEngine {
    class NativeScript {
    public:
        explicit NativeScript(Entity entity) : entity(entity) {}

        /// This doubles as `onDestroy()` because it is the only function guaranteed to run at destroy.
        virtual ~NativeScript() = default;

        NativeScript(const NativeScript& o) = delete;

        NativeScript& operator=(const NativeScript& o) = delete;

        NativeScript(NativeScript&& o) = delete;

        NativeScript& operator=(NativeScript&& o) = delete;

        // ------------------------------------ Lifecycle and Core Events ----------------------------------------------

        /// This function is called once at the initialization of the script.
        virtual void onStart() {}

        /// This function is called once per frame.
        virtual void onUpdate(float /*deltaTime*/) {}

        // ------------------------------------ Physics Events ---------------------------------------------------------

        /// This function is called if the entity has a **DyanmicRigidBodyComponent** and collides
        virtual void onCollisionEnter(const Entity& /*other*/) {}

        /// This function is called if the entity has a **DyanmicRigidBodyComponent** and exits a collision
        virtual void onCollisionExit(const Entity& /*other*/) {}

        // ------------------------------------ Additional Events ------------------------------------------------------

        /// This function is called if the entity has a **UIComponent** and the mouse cursor is over this entity.
        virtual void onMouseOver() {}

    protected:
        // ------------------------------------ Script Helper Functions ------------------------------------------------

        /**
         * Adds or overwrites the component of type <i>Component</i> to the entity this script belongs to.
         * @tparam Component
         * @tparam Args
         * @param args to be passed to the constructor of Component
         */
        template <typename Component, typename... Args>
        inline void setComponent(Args&&... args) {
            entity.setComponent<Component>(std::forward<Args>(args)...);
        }

        /**
         * Gets the component of type <i>Component</i> of the entity this script belongs to.
         * @tparam Component
         * @return Component
         */
        template <typename... Component>
        [[nodiscard]] decltype(auto) getComponent() {
            return entity.get<Component...>();
        }

        template <typename... Component>
        [[nodiscard]] decltype(auto) getComponent() const {
            return entity.get<Component...>();
        }

        /**
         * Check if the entity this script belongs to, has a component of type <i>Component</i>.
         * @tparam Component
         * @return bool
         */
        template <typename... Component>
        [[nodiscard]] decltype(auto) hasComponent() {
            return entity.has<Component...>();
        }

        // ------------------------------------ Input Helpers ----------------------------------------------------------
    protected:
        static bool isKeyDown(int keyCode);

        static bool isKeyUp(int keyCode);

        static bool isMouseButtonDown(int button);

        static bool isMouseButtonUp(int button);

    protected:
        Entity entity;
    };
}

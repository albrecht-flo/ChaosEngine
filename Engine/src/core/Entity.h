#pragma once

#include <entt/entity/registry.hpp>

struct Transform;

namespace ChaosEngine {
    /**
     * This is a handler to an entity for modifying associated data.
     *
     * See https://skypjack.github.io/entt/md_docs_md_entity.html for EnTT examples.
     */
    class Entity {
        friend class ECS;

    private:
        Entity(entt::registry* registry, entt::entity entity)
            : registry(registry), entity(entity) {
        }

    public:
        Entity() : registry(nullptr), entity(entt::null) {
        }

        ~Entity() = default;

        /**
         * Adds or overwrite the component of type <i>Component</i> to this entity.
         * @tparam Component
         * @tparam Args
         * @param args to be passed to the constructor of Component
         * @return Reference to the emplaced component
         */
        template <typename Component, typename... Args>
        inline decltype(auto) setComponent(Args&&... args) {
            assert("Registry must not be null" && registry != nullptr);
            return registry->emplace_or_replace<Component>(entity, std::forward<Args>(args)...);
        }

        /**
         * Get the component of type <i>Component</i> of this entity.
         * @tparam Component
         * @return Component
         */
        template <typename... Component>
        [[nodiscard]] decltype(auto) get() {
            assert("Registry must not be null" && registry != nullptr);
            return registry->get<Component...>(entity);
        }

        /**
         * @copydoc get
         */
        template <typename... Component>
        [[nodiscard]] decltype(auto) get() const {
            assert("Registry must not be null" && registry != nullptr);
            return registry->get<Component...>(entity);
        }

        /**
         * Try to a pointer to the component of type <i>Component</i> of this entity.
         * Returns nullptr if not found
         * @tparam Component
         * @return Component*
         */
        template <typename... Component>
        [[nodiscard]] decltype(auto) try_get() {
            assert("Registry must not be null" && registry != nullptr);
            return registry->try_get<Component...>(entity);
        }

        /**
         * @copydoc try_get
         */
        template <typename... Component>
        [[nodiscard]] decltype(auto) try_get() const {
            assert("Registry must not be null" && registry != nullptr);
            return registry->try_get<Component...>(entity);
        }

        /**
         * Check if the entity has a component of type <i>Component</i>.
         * @tparam Component
         * @return bool
         */
        template <typename... Component>
        [[nodiscard]] decltype(auto) has() {
            assert("Registry must not be null" && registry != nullptr);
            return registry->all_of<Component...>(entity);
        }

        explicit operator uint32_t() const {
            static_assert(std::is_same<entt::id_type, std::uint32_t>::value,
                          "Entity is not of type uint32_t so it can't be casted to it");
            return static_cast<uint32_t>(entity);
        }

        // Advanced Entity functions -----------------------------------------------------------------------------------

        void makeParentOf(Entity& entity);

        void makeChildOf(Entity& entity);

        // Advanced Entity operation functions -------------------------------------------------------------------------

        void move(const Transform& newTransform);

    private:
        friend class RigidBody2D;
        friend class SceneGraphSystem;

        entt::entity raw() { return entity; }

    private:
        entt::registry* registry;
        entt::entity entity;
    };
}

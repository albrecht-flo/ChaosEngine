#pragma once

#include <entt/entity/registry.hpp>

/**
 * This is a handler to an entity for modifying associated data.
 */
class Entity {
    friend class ECS;

private:
    Entity(entt::registry *registry, entt::entity entity)
            : registry(registry), entity(entity) {}

public:
    Entity() : entity(entt::null), registry(nullptr) {}

    ~Entity() = default;

    /**
     * Add or overwrite the component of type <i>Component</i> to this entity.
     * @tparam Component
     * @tparam Args
     * @param args to be passed to the constructor of Component
     */
    template<typename Component, typename ... Args>
    inline void setComponent(Args &&...args) {
        assert("Registry must not be null" && registry != nullptr);
        registry->emplace_or_replace<Component>(entity, std::forward<Args>(args)...);
    }

    /**
     * Get the component of type <i>Component</i> of this entity.
     * @tparam Component
     * @return
     */
    template<typename... Component>
    [[nodiscard]] decltype(auto) get() {
        assert("Registry must not be null" && registry != nullptr);
        return registry->get<Component...>(entity);
    }

    explicit  operator uint32_t() const {
        static_assert(std::is_same<entt::id_type, std::uint32_t>::value,
                      "Entity is not of type uint32_t so it can't be casted to it");
        return static_cast<uint32_t>(entity);
    }

private:
    entt::entity entity;
    entt::registry *registry;
};




#pragma once

#include <entt/entity/registry.hpp>

class Entity {
    friend class ECS;

private:
    Entity(entt::registry &registry, entt::entity entity) : registry(registry), entity(entity) {}

public:
    ~Entity() = default;

    template<typename Component, typename ... Args>
    inline void setComponent(Args &&...args) {
        registry.emplace_or_replace<Component>(entity, std::forward<Args>(args)...);
    }

    template<typename... Component>
    [[nodiscard]] decltype(auto) get() { return registry.get<Component...>(entity); }

private:
    entt::entity entity;
    entt::registry &registry;
};




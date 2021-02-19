#pragma once

#include <entt/entity/registry.hpp>
#include "Entity.h"

class ECS {
public:
    ECS() : registry() {}

    ~ECS() = default;

    ECS(const ECS &o) = delete;

    ECS &operator=(const ECS &o) = delete;

    ECS(ECS &&o) noexcept: registry(std::move(o.registry)) {}

    ECS &operator=(ECS &&o) noexcept {
        if (this == &o)
            return *this;
        registry = std::move(o.registry);
        return *this;
    }

    inline Entity createEntity() { return Entity(&registry, registry.create()); };

    inline entt::registry &getRegistry() { return registry; }

private:
    entt::registry registry;
};





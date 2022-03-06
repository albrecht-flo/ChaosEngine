#pragma once

#include <entt/entity/registry.hpp>
#include "Entity.h"

/**
 * Thin handle to the entity registry.
 */
class ECS {
public:
    using entity_t = uint32_t;
    static_assert(std::is_same<entity_t, std::underlying_type<entt::entity>::type>::value,
                  "EnTT entity type does not match engine entity type!");

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

    /// Create a new entity handle from this registry
    inline Entity createEntity() { return Entity{&registry, registry.create()}; };

    /// Create a new entity handle from this registry
    inline Entity getEntity(entity_t entity) {
        return Entity{&registry, registry.entity(static_cast<entt::entity>(entity))};
    };

    /// Access the internal registry
    inline entt::registry &getRegistry() { return registry; }

    inline const entt::registry &getRegistry() const { return registry; }

private:
    entt::registry registry;
};





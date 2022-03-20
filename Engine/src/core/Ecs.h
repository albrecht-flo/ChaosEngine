#pragma once

#include <stdexcept>
#include <entt/entity/registry.hpp>
#include <string>
#include "Entity.h"

namespace ChaosEngine {

    /**
     * Thin handle to the entity registry.
     */
    class ECS {
    public:
        using entity_t = entt::entity;

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

        /// Add a new entity to this registry
        inline Entity addEntity() { return Entity{&registry, registry.create()}; };

        /// Remove an entity from this registry
        inline void removeEntity(entity_t entity) { registry.destroy(entity); };

        /// Remove an entity from this registry
        inline void removeEntity(Entity entity) {
            if (!registry.valid(entity.entity)) {
                throw std::runtime_error(std::string("Tried to remove invalid entity") +
                                         std::to_string(static_cast<std::underlying_type<entt::entity>::type>(entity)));
            }
            registry.destroy(entity.entity);
        };

        /// Get an existing entity handle from this registry
        inline Entity getEntity(entity_t entity) {
            if (!registry.valid(entity)) {
                throw std::runtime_error(std::string("Tried to get invalid entity") +
                                         std::to_string(static_cast<std::underlying_type<entt::entity>::type>(entity)));
            }
            return Entity{&registry, entity};
        };

        /// Access the internal registry
        inline entt::registry &getRegistry() { return registry; }

        /// Const-Access to the internal registry
        inline const entt::registry &getRegistry() const { return registry; }

    public:
        static const entity_t null;

        static inline std::underlying_type<entt::entity>::type to_integral(entity_t entity) {
            return entt::to_integral(entity);
        }

    private:
        entt::registry registry;
    };

}

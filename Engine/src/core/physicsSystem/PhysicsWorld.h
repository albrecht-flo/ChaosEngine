#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "box2d/b2_world.h"

namespace ChaosEngine {

    class PhysicsWorld {
    public:
        explicit PhysicsWorld(const glm::vec2 &gravity);

        ~PhysicsWorld() = default;

        b2World &getPhysicsWorldRef() { return *physicsWorldRef; }

    private:
        std::unique_ptr<b2World> physicsWorldRef;
    };

}

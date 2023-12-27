#include "PhysicsWorld.h"

#include "Engine/src/core/utils/Logger.h"

using namespace ChaosEngine;

PhysicsWorld::PhysicsWorld(const glm::vec2 &gravity)
        : physicsWorldRef(std::make_unique<b2World>(b2Vec2{gravity.x, gravity.y})) {
    LOG_INFO("[PhysicsWorld] Init with gravity=({}, {})", gravity.x, gravity.y);
}

#include "PhysicsSystem2D.h"

namespace ChaosEngine {

    PhysicsSystem2D* PhysicsSystem2D::globalInstance = nullptr;

    PhysicsSystem2D::PhysicsSystem2D(): world(b2Vec2(0, -10)) {
        assert("Global physics2D instance is already set" && globalInstance == nullptr);
        globalInstance = this;
    }

    PhysicsSystem2D::~PhysicsSystem2D() {
        globalInstance = nullptr;
    }

    void PhysicsSystem2D::init(const SceneConfiguration&) {
    }

    void PhysicsSystem2D::update(const ECS&, float deltaTime) {
        world.Step(deltaTime, velocityIterations, positionIterations);
    }

}

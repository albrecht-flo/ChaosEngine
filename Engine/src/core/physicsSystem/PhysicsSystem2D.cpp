#include "PhysicsSystem2D.h"
#include "Engine/src/core/Ecs.h"

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

    void PhysicsSystem2D::update(ECS& ecs, float deltaTime) {
        world.Step(deltaTime, velocityIterations, positionIterations);

        auto view = ecs.getRegistry().view<Transform, DynamicRigidBodyComponent>();
        for (auto[entity, transform, body]: view.each()) {
            const Transform phyTransform = body.body.getTransform();
            transform.position.x = phyTransform.position.x;
            transform.position.y = phyTransform.position.y;
            transform.rotation.z = glm::degrees(phyTransform.rotation.z);
        }
    }

}

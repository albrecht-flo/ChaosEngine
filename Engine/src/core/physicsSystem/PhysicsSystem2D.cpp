#include "PhysicsSystem2D.h"
#include "Engine/src/core/Ecs.h"
#include "core/utils/Logger.h"
#include <box2d/b2_collision.h>
#include <box2d/b2_contact.h>

using namespace ChaosEngine;

PhysicsSystem2D::Physics2DCollisionListener::Physics2DCollisionListener(ECS& ecs): ecs(ecs) {}

PhysicsSystem2D::Physics2DCollisionListener::~Physics2DCollisionListener() = default;

void PhysicsSystem2D::Physics2DCollisionListener::BeginContact(b2Contact* contact) {
    // Logger::D("Physics2DCollisionListener", "BeginContact");

    const auto entityA = static_cast<entt::entity>(contact->GetFixtureA()->GetBody()->GetUserData().pointer);
    const auto entityB = static_cast<entt::entity>(contact->GetFixtureB()->GetBody()->GetUserData().pointer);
    // LOG_DEBUG("Checking for entity {} and {}", entityA, entityB);

    if (ecs.getRegistry().any_of<NativeScriptComponent>(entityA)) {
        const auto other = Entity(ecs.getEntity(entityB));
        ecs.getRegistry().get<NativeScriptComponent>(entityA).script->onCollisionEnter(other);
    }
    if (ecs.getRegistry().any_of<NativeScriptComponent>(entityB)) {
        const auto other = Entity(ecs.getEntity(entityA));
        ecs.getRegistry().get<NativeScriptComponent>(entityB).script->onCollisionEnter(other);
    }
}

void PhysicsSystem2D::Physics2DCollisionListener::EndContact(b2Contact* contact) {
    // Logger::D("Physics2DCollisionListener", "EndContact");

    const auto entityA = static_cast<entt::entity>(contact->GetFixtureA()->GetBody()->GetUserData().pointer);
    const auto entityB = static_cast<entt::entity>(contact->GetFixtureB()->GetBody()->GetUserData().pointer);

    // This check can fail when one of the entities got destroyed
    if (ecs.getRegistry().valid(entityA) && ecs.getRegistry().valid(entityB)) {
        if (ecs.getRegistry().any_of<NativeScriptComponent>(entityA)) {
            const auto other = Entity(ecs.getEntity(entityB));
            ecs.getRegistry().get<NativeScriptComponent>(entityA).script->onCollisionExit(other);
        }
        if (ecs.getRegistry().any_of<NativeScriptComponent>(entityB)) {
            const auto other = Entity(ecs.getEntity(entityA));
            ecs.getRegistry().get<NativeScriptComponent>(entityB).script->onCollisionExit(other);
        }
    }
}

void PhysicsSystem2D::Physics2DCollisionListener::PreSolve(b2Contact*,
                                                           const b2Manifold*) {}

void PhysicsSystem2D::Physics2DCollisionListener::PostSolve(b2Contact*,
                                                            const b2ContactImpulse*) {}

PhysicsSystem2D* PhysicsSystem2D::globalInstance = nullptr;

PhysicsSystem2D::PhysicsSystem2D(): world(b2Vec2(0, -10)) {
    assert("Global physics2D instance is already set" && globalInstance == nullptr);
    globalInstance = this;
}

PhysicsSystem2D::~PhysicsSystem2D() {
    globalInstance = nullptr;
}

void PhysicsSystem2D::init(const SceneConfiguration&, ECS& ecs) {
    collusionListener = std::make_unique<Physics2DCollisionListener>(ecs);
    world.SetContactListener(collusionListener.get());
}

void PhysicsSystem2D::update(ECS& ecs, float deltaTime) {
    world.Step(deltaTime, velocityIterations, positionIterations);

    auto view = ecs.getRegistry().view<Transform, DynamicRigidBodyComponent>();
    for (auto [entity, transform, body] : view.each()) {
        const Transform phyTransform = body.body.getTransform();
        transform.position.x = phyTransform.position.x;
        transform.position.y = phyTransform.position.y;
        transform.rotation.z = glm::degrees(phyTransform.rotation.z);
    }
}

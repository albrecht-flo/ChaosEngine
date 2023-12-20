#include "Physics2DBody.h"
#include "PhysicsSystem2D.h"
#include "core/Components.h"

#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>

namespace ChaosEngine {
  void Physics2DBody::destroy() {
    PhysicsSystem2D::globalInstance->destroyBody(body);
#ifndef NDEBUG
    body = nullptr;
#endif
  }

  StaticRigidBodyComponent RigidBody2D::CreateStaticRigidBody(const Entity& entity, const Transform& transform) {
    b2BodyDef def{};
    def.position.Set(transform.position.x, transform.position.y);
    def.angle = glm::radians(transform.rotation.z);
    def.type = b2_staticBody;
    def.userData.pointer = static_cast<uintptr_t>(entity.entity);

    auto body = PhysicsSystem2D::globalInstance->createBody(def);

    // TODO: multiple shapes
    b2PolygonShape shape;
    shape.SetAsBox(transform.scale.x, transform.scale.y);

    b2FixtureDef fixtureDef{};
    fixtureDef.shape = &shape;
    fixtureDef.density = 0.0f;

    body.CreateFixture(fixtureDef);
    return StaticRigidBodyComponent{.body = std::move(body)};
  }

  DynamicRigidBodyComponent RigidBody2D::CreateDynamicRigidBody(const Entity& entity, const Transform& transform, float density, float friction) {
    b2BodyDef def{};
    def.position.Set(transform.position.x, transform.position.y);
    def.angle = glm::radians(transform.rotation.z);
    def.type = b2_dynamicBody;
    def.userData.pointer = static_cast<uintptr_t>(entity.entity);

    auto body = PhysicsSystem2D::globalInstance->createBody(def);

    // TODO: multiple shapes
    b2PolygonShape shape;
    shape.SetAsBox(transform.scale.x, transform.scale.y);

    b2FixtureDef fixtureDef{};
    fixtureDef.shape = &shape;
    fixtureDef.density = density;
    fixtureDef.friction = friction;

    body.CreateFixture(fixtureDef);
    return DynamicRigidBodyComponent{.body = std::move(body)};
  }
}

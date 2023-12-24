#include "Physics2DBody.h"
#include "PhysicsSystem2D.h"
#include "core/Components.h"

#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

namespace ChaosEngine {
  void Physics2DBody::destroy() {
    if (body != nullptr)
      PhysicsSystem2D::globalInstance->destroyBody(body);
#ifndef NDEBUG
    body = nullptr;
#endif
  }

  Transform Physics2DBody::getTransform() const {
    const auto position = body->GetPosition();
    return Transform{
      .position = glm::vec3(position.x, position.y, 0),
      .rotation = glm::vec3(0, 0, body->GetAngle()),
      .scale = glm::vec3()
    };
  }

  void Physics2DBody::setVelocity(const glm::vec3& velocity) {
    body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
  }


  static std::unique_ptr<b2Shape> getBox2DShape(const RigidBody2D::RigitBody2DShape& shape) {
    constexpr float skin = b2_polygonRadius / 2;
    switch (shape.type) {
    case RigidBody2D::Box: {
      auto ret = std::make_unique<b2PolygonShape>();
      ret->SetAsBox(shape.dimension.x - skin, shape.dimension.y - skin);
      return ret;
    }
    case RigidBody2D::Cricle: {
      auto ret = std::make_unique<b2CircleShape>();
      ret->m_p.SetZero();
      ret->m_radius = shape.dimension.x - skin;
      return ret;
    }
    default:
      assert("Unsupported 2D Shape" && false);
      break;
    }
    return nullptr;
  }

  StaticRigidBodyComponent RigidBody2D::CreateStaticRigidBody(Entity& entity, const RigitBody2DShape& shapeDef) {
    // LOG_DEBUG("Creating StaticRigidBody2D for entity {}", static_cast<uint32_t>(entity));
    const auto transform = entity.get<Transform>();

    b2BodyDef def{};
    def.position.Set(transform.position.x, transform.position.y);
    def.angle = glm::radians(transform.rotation.z);
    def.type = b2_staticBody;
    def.userData.pointer = static_cast<uintptr_t>(entity.entity);

    auto body = PhysicsSystem2D::globalInstance->createBody(def);

    auto shape = getBox2DShape(shapeDef);

    b2FixtureDef fixtureDef{};
    fixtureDef.shape = shape.get();
    fixtureDef.density = 0.0f;

    body.CreateFixture(fixtureDef);
    return StaticRigidBodyComponent{.body = std::move(body)};
  }

  DynamicRigidBodyComponent RigidBody2D::CreateDynamicRigidBody(Entity& entity, const RigitBody2DShape& shapeDef,
                                                                float density, float friction, bool useGravity) {
    // LOG_DEBUG("Creating DynamicRigidBody2D for entity {}", static_cast<uint32_t>(entity));
    const auto transform = entity.get<Transform>();
    b2BodyDef def{};
    def.position.Set(transform.position.x, transform.position.y);
    def.angle = glm::radians(transform.rotation.z);
    def.type = b2_dynamicBody;
    def.userData.pointer = static_cast<uintptr_t>(entity.entity);
    def.gravityScale = useGravity ? 1.0f : 0.0f;

    auto body = PhysicsSystem2D::globalInstance->createBody(def);

    auto shape = getBox2DShape(shapeDef);

    b2FixtureDef fixtureDef{};
    fixtureDef.shape = shape.get();
    fixtureDef.density = density;
    fixtureDef.friction = friction;

    body.CreateFixture(fixtureDef);
    return DynamicRigidBodyComponent{.body = std::move(body)};
  }
}

#pragma once
#include <box2d/b2_body.h>

struct StaticRigidBodyComponent;
struct DynamicRigidBodyComponent;
struct Transform;

namespace ChaosEngine {
  class Entity;

  class Physics2DBody {
  public:
    explicit Physics2DBody(b2Body* body) : body(body) {
    }

    ~Physics2DBody() { destroy(); }

    // Delete copy construction and assignment
    Physics2DBody(const Physics2DBody& o) = delete;
    Physics2DBody& operator=(const Physics2DBody& o) = delete;

    // Implement move construction and assignment
    Physics2DBody(Physics2DBody&& o) noexcept: body(o.body) {
    }

    Physics2DBody& operator=(Physics2DBody&& o) noexcept {
      if (this == &o) return *this;
      destroy();
      body = o.body;
      return *this;
    }

    b2Fixture* CreateFixture(const b2FixtureDef& def) { return body->CreateFixture(&def); }

  private:
    void destroy();
    b2Body* body;
  };


  class RigidBody2D {
  public:
    static StaticRigidBodyComponent CreateStaticRigidBody(const Entity& entity, const Transform& transform);
    static DynamicRigidBodyComponent CreateDynamicRigidBody(const Entity& entity, const Transform& transform, float density, float friction);
  };
}

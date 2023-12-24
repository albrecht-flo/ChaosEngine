#pragma once
#include <vector>
#include <box2d/b2_body.h>
#include <glm/glm.hpp>

struct StaticRigidBodyComponent;
struct DynamicRigidBodyComponent;
struct Transform;

namespace ChaosEngine {
  class Entity;

  class Physics2DBody {
  public:
    explicit Physics2DBody(b2Body* body) : body(body) {}

    ~Physics2DBody() { destroy(); }

    // Delete copy construction and assignment
    Physics2DBody(const Physics2DBody& o) = delete;
    Physics2DBody& operator=(const Physics2DBody& o) = delete;

    // Implement move construction and assignment
    Physics2DBody(Physics2DBody&& o) noexcept: body(o.body) {
      o.body = nullptr;
    }

    Physics2DBody& operator=(Physics2DBody&& o) noexcept {
      if (this == &o) return *this;
      destroy();
      body = o.body;
      o.body = nullptr;
      return *this;
    }

    [[nodiscard]] Transform getTransform() const;

    void setVelocity(const glm::vec3& velocity);

  private:
    friend class RigidBody2D;
    b2Fixture* CreateFixture(const b2FixtureDef& def) { return body->CreateFixture(&def); }

  private:
    void destroy();
    b2Body* body;
  };


  class RigidBody2D {
  public:
    enum RigitBody2DShapeType { Cricle, Line, Box, Polygon };

    struct RigitBody2DShape {
      RigitBody2DShapeType type;
      glm::vec2 dimension;
      std::vector<glm::vec2> vertices;
    };

  public:
    static StaticRigidBodyComponent CreateStaticRigidBody(Entity& entity, const RigitBody2DShape& shape);
    static DynamicRigidBodyComponent CreateDynamicRigidBody(Entity& entity, const RigitBody2DShape& shape,
                                                            float density, float friction, bool useGravity = true);
  };
}

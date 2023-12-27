#include "PhysicsSystem2D.h"
#include "Engine/src/core/Ecs.h"
#include "core/utils/Logger.h"
#include "renderer/api/RendererAPI.h"
#include <box2d/b2_collision.h>
#include <box2d/b2_contact.h>

using namespace ChaosEngine;

PhysicsSystem2D::Physics2DCollisionListener::Physics2DCollisionListener(ECS &ecs) : ecs(ecs) {}

PhysicsSystem2D::Physics2DCollisionListener::~Physics2DCollisionListener() = default;

void PhysicsSystem2D::Physics2DCollisionListener::BeginContact(b2Contact *contact) {
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

void PhysicsSystem2D::Physics2DCollisionListener::EndContact(b2Contact *contact) {
    // Logger::D("Physics2DCollisionListener", "EndContact");

    const auto entityA = static_cast<entt::entity>(contact->GetFixtureA()->GetBody()->GetUserData().pointer);
    const auto entityB = static_cast<entt::entity>(contact->GetFixtureB()->GetBody()->GetUserData().pointer);

    // This check can fail when one of the entities got destroyed TODO: Fix this propperly
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

void PhysicsSystem2D::Physics2DCollisionListener::PreSolve(b2Contact *,
                                                           const b2Manifold *) {}

void PhysicsSystem2D::Physics2DCollisionListener::PostSolve(b2Contact *,
                                                            const b2ContactImpulse *) {}

// ---------------------------------------------------------------------------------------------------------------------
PhysicsSystem2D::Physics2DDebugDraw::Physics2DDebugDraw() : data(std::make_shared<Renderer::DebugRenderData>()) {}

void PhysicsSystem2D::Physics2DDebugDraw::tick() {
    data->points.clear();
    data->lines.clear();
    data->triangles.clear();
}

void
PhysicsSystem2D::Physics2DDebugDraw::DrawPolygon(const b2Vec2 *vertices, int32_t vertexCount, const b2Color &color) {
//    LOG_DEBUG("Physics2DDebugDraw::DrawPolygon");

    b2Vec2 p1 = vertices[vertexCount - 1];
    for (int32 i = 0; i < vertexCount; ++i) {
        b2Vec2 p2 = vertices[i];
        data->lines.emplace_back(VertexPCU{.pos{p1.x, p1.y, 1}, .color{0, 1, 0.68f, 1}, .uv{}});
        data->lines.emplace_back(VertexPCU{.pos{p2.x, p2.y, 1}, .color{0, 1, 0.68f, 1}, .uv{}});
        p1 = p2;
    }
}

void PhysicsSystem2D::Physics2DDebugDraw::DrawSolidPolygon(const b2Vec2 *vertices, int32 vertexCount,
                                                           const b2Color &color) {

    //LOG_DEBUG("Physics2DDebugDraw::DrawSolidPolygon");

    b2Vec2 p1 = vertices[vertexCount - 1];
    for (int32 i = 0; i < vertexCount; ++i) {
        b2Vec2 p2 = vertices[i];
        data->lines.emplace_back(VertexPCU{.pos{p1.x, p1.y, 1}, .color{0, 1, 0.68f, 1}, .uv{}});
        data->lines.emplace_back(VertexPCU{.pos{p2.x, p2.y, 1}, .color{0, 1, 0.68f, 1}, .uv{}});
        p1 = p2;
    }
}

void PhysicsSystem2D::Physics2DDebugDraw::DrawCircle(const b2Vec2 &center, float radius, const b2Color &color) {
    const float k_segments = 16.0f;
    const float k_increment = 2.0f * b2_pi / k_segments;
    float sinInc = sinf(k_increment);
    float cosInc = cosf(k_increment);
    b2Vec2 r1(1.0f, 0.0f);
    b2Vec2 v1 = center + radius * r1;
    for (int32 i = 0; i < k_segments; ++i) {
        // Perform rotation to avoid additional trigonometry.
        b2Vec2 r2{};
        r2.x = cosInc * r1.x - sinInc * r1.y;
        r2.y = sinInc * r1.x + cosInc * r1.y;
        b2Vec2 v2 = center + radius * r2;
        data->lines.emplace_back(VertexPCU{.pos{v1.x, v1.y, 1}, .color{0, 1, 0.68f, 1}, .uv{}});
        data->lines.emplace_back(VertexPCU{.pos{v2.x, v2.y, 1}, .color{0, 1, 0.68f, 1}, .uv{}});
        r1 = r2;
        v1 = v2;
    }
}

void PhysicsSystem2D::Physics2DDebugDraw::DrawSolidCircle(const b2Vec2 &center, float radius, const b2Vec2 &axis,
                                                          const b2Color &color) {
    const float k_segments = 16.0f;
    const float k_increment = 2.0f * b2_pi / k_segments;
    float sinInc = sinf(k_increment);
    float cosInc = cosf(k_increment);
    b2Vec2 r1(1.0f, 0.0f);
    b2Vec2 v1 = center + radius * r1;
    for (int32 i = 0; i < k_segments; ++i) {
        // Perform rotation to avoid additional trigonometry.
        b2Vec2 r2{};
        r2.x = cosInc * r1.x - sinInc * r1.y;
        r2.y = sinInc * r1.x + cosInc * r1.y;
        b2Vec2 v2 = center + radius * r2;
        data->lines.emplace_back(VertexPCU{.pos{v1.x, v1.y, 1}, .color{0, 1, 0.68f, 1}, .uv{}});
        data->lines.emplace_back(VertexPCU{.pos{v2.x, v2.y, 1}, .color{0, 1, 0.68f, 1}, .uv{}});
        r1 = r2;
        v1 = v2;
    }
    b2Vec2 p = center + radius * axis;
    data->lines.emplace_back(VertexPCU{.pos{center.x, center.y, 1}, .color{0.66f, 0, 0, 1}, .uv{}});
    data->lines.emplace_back(VertexPCU{.pos{p.x, p.y, 1}, .color{0.66f, 0, 0, 1}, .uv{}});
}

void PhysicsSystem2D::Physics2DDebugDraw::DrawSegment(const b2Vec2 &p1, const b2Vec2 &p2, const b2Color &color) {
    data->lines.emplace_back(VertexPCU{.pos{p1.x, p1.y, 1}, .color{0, 1, 0.68f, 1}, .uv{}});
    data->lines.emplace_back(VertexPCU{.pos{p2.x, p2.y, 1}, .color{0, 1, 0.68f, 1}, .uv{}});
}

void PhysicsSystem2D::Physics2DDebugDraw::DrawTransform(const b2Transform &xf) {}

void PhysicsSystem2D::Physics2DDebugDraw::DrawPoint(const b2Vec2 &p, float size, const b2Color &color) {
    data->points.emplace_back(VertexPCU{.pos{p.x, p.y, 1}, .color{1, 1, 0, 1}, .uv{}});
    data->points.emplace_back(VertexPCU{.pos{p.x, p.y, 1}, .color{1, 1, 0, 1}, .uv{}});
}

// ---------------------------------------------------------------------------------------------------------------------

PhysicsSystem2D *PhysicsSystem2D::globalInstance = nullptr;

PhysicsSystem2D::PhysicsSystem2D() : world(b2Vec2(0, -10)) {
    assert("Global physics2D instance is already set" && globalInstance == nullptr);
    globalInstance = this;
}

PhysicsSystem2D::~PhysicsSystem2D() {
    globalInstance = nullptr;
}

void PhysicsSystem2D::init(const SceneConfiguration &, ECS &ecs) {
    collusionListener = std::make_unique<Physics2DCollisionListener>(ecs);
    world.SetContactListener(collusionListener.get());

    debugDrawer = std::make_unique<Physics2DDebugDraw>();
    debugDrawer->SetFlags(b2Draw::e_shapeBit);
    world.SetDebugDraw(debugDrawer.get());
}

void PhysicsSystem2D::update(ECS &ecs, float deltaTime) {
    world.Step(deltaTime, velocityIterations, positionIterations);

    auto view = ecs.getRegistry().view<Transform, DynamicRigidBodyComponent>();
    for (auto [entity, transform, body]: view.each()) {
        const Transform phyTransform = body.body.getTransform();
        transform.position.x = phyTransform.position.x;
        transform.position.y = phyTransform.position.y;
        transform.rotation.z = glm::degrees(phyTransform.rotation.z);
    }
}

std::shared_ptr<Renderer::DebugRenderData> PhysicsSystem2D::getDebugData() {
    debugDrawer->tick();
    world.DebugDraw();
    return debugDrawer->getData();
}

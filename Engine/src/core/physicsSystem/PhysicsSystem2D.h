#pragma once

#include <box2d/b2_world.h>
#include <box2d/b2_draw.h>

#include <core/Scene.h>
#include "Physics2DBody.h"

struct StaticRigidBodyComponent;
struct DynamicRigidBodyComponent;

namespace Renderer {
    struct DebugRenderData;
}

namespace ChaosEngine {

    class PhysicsSystem2D {
        class Physics2DCollisionListener : public b2ContactListener {
        public:
            Physics2DCollisionListener(ECS &ecs);

            ~Physics2DCollisionListener();

            void BeginContact(b2Contact *contact) override;

            void EndContact(b2Contact *contact) override;

            void PreSolve(b2Contact *contact, const b2Manifold *oldManifold) override;

            void PostSolve(b2Contact *contact, const b2ContactImpulse *impulse) override;

        private:
            ECS &ecs;
        };

        class Physics2DDebugDraw : public b2Draw {
        public:
            Physics2DDebugDraw();

            ~Physics2DDebugDraw() override = default;


            /// Draw a closed polygon provided in CCW order.
            void DrawPolygon(const b2Vec2 *vertices, int32 vertexCount, const b2Color &color) override;

            /// Draw a solid closed polygon provided in CCW order.
            void DrawSolidPolygon(const b2Vec2 *vertices, int32 vertexCount, const b2Color &color) override;

            /// Draw a circle.
            void DrawCircle(const b2Vec2 &center, float radius, const b2Color &color) override;

            /// Draw a solid circle.
            void DrawSolidCircle(const b2Vec2 &center, float radius, const b2Vec2 &axis, const b2Color &color) override;

            /// Draw a line segment.
            void DrawSegment(const b2Vec2 &p1, const b2Vec2 &p2, const b2Color &color) override;

            /// Draw a transform. Choose your own length scale.
            /// @param xf a transform.
            void DrawTransform(const b2Transform &xf) override;

            /// Draw a point.
            void DrawPoint(const b2Vec2 &p, float size, const b2Color &color) override;

            void tick();

            std::shared_ptr<Renderer::DebugRenderData> getData() { return data; }

        private:
            std::shared_ptr<Renderer::DebugRenderData> data;
        };

    public:
        PhysicsSystem2D();

        ~PhysicsSystem2D();

        void init(const SceneConfiguration &config, ECS &ecs);

        void update(ECS &ecs, float deltaTime);

        std::shared_ptr<Renderer::DebugRenderData> getDebugData();

    private:
        friend Physics2DBody;
        friend RigidBody2D;
        static PhysicsSystem2D *globalInstance;

        void destroyBody(b2Body *body) {
            if (body == nullptr) return;
            world.DestroyBody(body);
        }

        Physics2DBody createBody(const b2BodyDef &def) { return Physics2DBody{world.CreateBody(&def)}; }

    private:
        b2World world;
        std::unique_ptr<Physics2DCollisionListener> collusionListener = nullptr;
        std::unique_ptr<Physics2DDebugDraw> debugDrawer = nullptr;

        // Based on recommended values of Box2D
        // See: https://box2d.org/documentation/md__d_1__git_hub_box2d_docs_hello.html#autotoc_md24
        const int32_t velocityIterations = 8;
        const int32_t positionIterations = 3;
    };
} // ChaosEngine

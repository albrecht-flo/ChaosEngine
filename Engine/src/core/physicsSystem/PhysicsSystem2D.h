#pragma once
#include <core/Scene.h>

#include <box2d/b2_world.h>
#include "Physics2DBody.h"

struct StaticRigidBodyComponent;
struct DynamicRigidBodyComponent;

namespace ChaosEngine {

    class PhysicsSystem2D {
    public:
        PhysicsSystem2D();
        ~PhysicsSystem2D();

        void init(const SceneConfiguration& config);
        void update(ECS& ecs, float deltaTime);

    private:
        friend Physics2DBody;
        friend RigidBody2D;
        static PhysicsSystem2D* globalInstance;

        void destroyBody(b2Body* body) {
            if (body == nullptr) return;
            world.DestroyBody(body);
        }

        Physics2DBody createBody(const b2BodyDef& def) { return Physics2DBody{world.CreateBody(&def)}; }

        void registerBody();

    private:
        b2World world;

        // Based on recommended values of Box2D
        // See: https://box2d.org/documentation/md__d_1__git_hub_box2d_docs_hello.html#autotoc_md24
        const int32_t velocityIterations = 8;
        const int32_t positionIterations = 3;
    };

} // ChaosEngine

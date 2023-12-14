#pragma once
#include <core/Scene.h>

#include <box2d/b2_world.h>

namespace ChaosEngine {

class PhysicsSystem2D {
public:
    PhysicsSystem2D();
    ~PhysicsSystem2D();

    void init(const SceneConfiguration& config);
    void update(const ECS& ecs, float deltaTime);
private:
    b2World world;

    // Based on recommended values of Box2D
    // See: https://box2d.org/documentation/md__d_1__git_hub_box2d_docs_hello.html#autotoc_md24
    const int32_t velocityIterations = 8;
    const int32_t positionIterations = 3;
};

} // ChaosEngine

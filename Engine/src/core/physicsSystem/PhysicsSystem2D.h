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
};

} // ChaosEngine

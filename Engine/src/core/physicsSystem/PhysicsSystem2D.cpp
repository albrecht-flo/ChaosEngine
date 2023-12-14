//
// Created by flo on 13.12.23.
//

#include "PhysicsSystem2D.h"

namespace ChaosEngine
{
    PhysicsSystem2D::PhysicsSystem2D(): world(b2Vec2(0, -10))
    {
    }

    PhysicsSystem2D::~PhysicsSystem2D()
    {

    }

    void PhysicsSystem2D::init(const SceneConfiguration& config)
    {
    }

    void PhysicsSystem2D::update(const ECS& ecs, float deltaTime)
    {
        world.Step(deltaTime, velocityIterations, positionIterations);
    }
}

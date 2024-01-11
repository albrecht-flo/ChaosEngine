#pragma once

#include "Engine/src/core/Scene.h"

namespace ChaosEngine {

    class SceneGraphSystem {
    public:
        SceneGraphSystem();

        ~SceneGraphSystem();

        SceneGraphSystem(const SceneGraphSystem &o) = delete;

        SceneGraphSystem &operator=(const SceneGraphSystem &o) = delete;

        SceneGraphSystem(SceneGraphSystem &&o) = delete;

        SceneGraphSystem &operator=(SceneGraphSystem &&o) = delete;

        void init(Scene &scene);

        // -------------------------------------------------------------------------------------------------------------

        static void CreateParentChildRelationship(Entity &parent, Entity &child);

        static void UpdateTransformWithChildren(Entity &entity);

    private:
        static SceneGraphSystem* sceneGraphSystemInstance;
    };

}


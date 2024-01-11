#include "Entity.h"

#include "Components.h"
#include "sceneGraphSystem/SceneGraphSystem.h"

using namespace ChaosEngine;


void Entity::makeParentOf(Entity &child) {
    SceneGraphSystem::CreateParentChildRelationship(*this, child);
}

void Entity::makeChildOf(Entity &parent) {
    SceneGraphSystem::CreateParentChildRelationship(parent, *this);
}

void Entity::move(const Transform &newTransform) {
    auto *sgc = try_get<SceneGraphComponent>();
    if (sgc == nullptr) {
        setComponent<Transform>(newTransform);
    } else {
        sgc->localTransform = newTransform;
        SceneGraphSystem::UpdateTransformWithChildren(*this);
    }

}
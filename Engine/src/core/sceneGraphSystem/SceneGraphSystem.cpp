#include "SceneGraphSystem.h"

#include "core/Entity.h"
#include "core/Components.h"

using namespace ChaosEngine;

SceneGraphSystem *SceneGraphSystem::sceneGraphSystemInstance = nullptr;

SceneGraphSystem::SceneGraphSystem() {
    if (sceneGraphSystemInstance == nullptr)
        sceneGraphSystemInstance = this;
    else {
        assert("Only one sceneGraphSystem can be created" && false);
    }
}

SceneGraphSystem::~SceneGraphSystem() {

}

static void destroyFunc(entt::registry &registry, entt::entity entity) {
    auto &sgc = registry.get<SceneGraphComponent>(entity);
    LOG_DEBUG("Destroying scene graph component {}", sgc.next);

    if (sgc.firstChild != entt::null) { // Parent node
        // remove parent relation from children -> yeet to layer 0
        entt::entity cur = sgc.firstChild;
        SceneGraphComponent *curSGC = registry.try_get<SceneGraphComponent>(cur);
        do {
            // Make global transform local transform
            curSGC->localTransform = registry.get<Transform>(cur);
            cur = curSGC->next;
            // Move to layer 0 and remove linked list under current entity (parent)
            curSGC->parent = entt::null;
            curSGC->next = entt::null;
            curSGC->prev = entt::null;
            curSGC = registry.try_get<SceneGraphComponent>(cur);
        } while (cur != sgc.firstChild);
    }

    // Remove from list, NOP if only 1 element -> we don't care because the first/last cases are covered below
    if (sgc.prev != entt::null && sgc.next != entt::null) {
        SceneGraphComponent *prevSGC = registry.try_get<SceneGraphComponent>(sgc.prev);
        SceneGraphComponent *nextSGC = registry.try_get<SceneGraphComponent>(sgc.next);
        prevSGC->next = sgc.next;
        nextSGC->prev = sgc.prev;
    }

    // Clear up if this entity is the first or last element in the list
    if (sgc.parent != entt::null) {
        SceneGraphComponent *parentSGC = registry.try_get<SceneGraphComponent>(sgc.parent);
        if (parentSGC->firstChild == entity) { // First in list
            if (sgc.next != entity) { // List has more than 1 element
                parentSGC->firstChild = sgc.next;
            } else {
                parentSGC->firstChild = entt::null;
            }
        }
        if (parentSGC->lastChild == entity) { // Last in list
            if (sgc.prev != entity) { // List has more than 1 element
                parentSGC->lastChild = sgc.prev;
            } else {
                parentSGC->lastChild = entt::null;
            }
        }
    }
}

void SceneGraphSystem::init(Scene &scene) {
    scene.getECS().getRegistry().on_destroy<SceneGraphComponent>().connect<&destroyFunc>();
}


static void appendToParent(entt::registry *registry, entt::entity parent, entt::entity child) {
    //    LOG_DEBUG("CreateParentChildRelationship(parent={}, child={})", parent, child);
    auto &parentSGC = registry->get<SceneGraphComponent>(parent);
    auto &childSGC = registry->get<SceneGraphComponent>(child);

    if (parentSGC.firstChild == entt::null) {
        parentSGC.firstChild = child;
        parentSGC.lastChild = child;
    }

    // Get last child
    entt::entity lastChild = parentSGC.lastChild;
    // Init new child
    childSGC.parent = parent;
    childSGC.next = parentSGC.firstChild;
    childSGC.prev = lastChild;
    // Set end of list
    parentSGC.lastChild = child;
    // Update previously last element
    auto &lastSGC = registry->get<SceneGraphComponent>(lastChild);
    lastSGC.next = child;

    auto &firstSGC = registry->get<SceneGraphComponent>(parentSGC.firstChild);
    firstSGC.prev = child;
}

void SceneGraphSystem::CreateParentChildRelationship(Entity &parent, Entity &child) {
//    LOG_DEBUG("CreateParentChildRelationship(parent={}, child={})", parent.raw(), child.raw());
    assert(parent.has<Transform>() && child.has<Transform>());

    if (!parent.has<SceneGraphComponent>()) {
        parent.setComponent<SceneGraphComponent>(SceneGraphComponent{
                .localTransform = parent.get<Transform>(),
                .parent = entt::null,
                .firstChild = entt::null,
                .lastChild = entt::null,
                .next = entt::null,
                .prev = entt::null,
        });
    }

    if (!child.has<SceneGraphComponent>()) {
        // Create
        child.setComponent<SceneGraphComponent>(SceneGraphComponent{
                .localTransform{glm::vec3{0}, glm::vec3{0}, glm::vec3{1}},
                .parent = entt::null,
                .firstChild = entt::null,
                .lastChild = entt::null,
                .next = entt::null,
                .prev = entt::null,
        });
    }
    appendToParent(parent.registry, parent.raw(), child.raw());

    // Update transforms
    UpdateTransformWithChildren(parent);
}

void SceneGraphSystem::UpdateTransformWithChildren(Entity &entity) {
    assert(entity.has<SceneGraphComponent>());

    auto &sgc = entity.get<SceneGraphComponent>();
    auto *registry = entity.registry;

    // Compute own global transform from parent
    if (sgc.parent == entt::null)
        entity.setComponent<Transform>(sgc.localTransform);
    else {
        const Transform &parentT = registry->get<Transform>(sgc.parent);
        entity.setComponent<Transform>(sgc.localTransform.transform(parentT));
    }

    // Update all children
    if (sgc.firstChild != entt::null) {
        entt::entity cur = sgc.firstChild;
        SceneGraphComponent *curSGC = registry->try_get<SceneGraphComponent>(cur);
        do {
            auto tmp = Entity{registry, cur};
            UpdateTransformWithChildren(tmp);
            cur = curSGC->next;
            curSGC = registry->try_get<SceneGraphComponent>(cur);
        } while (cur != sgc.firstChild);
    }
}

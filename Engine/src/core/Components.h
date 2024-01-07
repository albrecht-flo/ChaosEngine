#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm defaults to opengl depth -1 to 1, Vulkan is using 0 to 1

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>
#include <utility>
#include <memory>

#include "Engine/src/renderer/api/Material.h"
#include "Engine/src/renderer/api/RenderMesh.h"
#include "Engine/src/core/scriptSystem/NativeScript.h"
#include "Engine/src/core/assets/Font.h"
#include "Engine/src/core/physicsSystem/Physics2DBody.h"
#include "Engine/src/core/audioSystem/api/AudioSource.h"


namespace ChaosEngine {
    class RenderingSystem;

    class SceneGraphSystem;

    class NativeScriptSystem;
}

struct Meta {
    std::string name;
};


struct Transform {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    inline glm::mat4 getModelMatrix() const {
        glm::mat4 ret = glm::translate(glm::mat4(1), position);
        ret *= glm::toMat4(glm::quat({glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)}));
        return glm::scale(ret, scale);;
    }
};

// ---------------------------------------------------------------------------------------------------------------------

struct TransformComponent {
    Transform local; // To be set from scripts
    explicit TransformComponent(const Transform &transform) : local(transform), global(transform),
                                                              parent(entt::null), needsGlobalRecalculation(false) {}

    TransformComponent(const Transform &transform, entt::entity parent) : local(transform), global(Transform{}),
                                                                          parent(parent),
                                                                          needsGlobalRecalculation(true) {}

private: // Internals
    friend class ChaosEngine::RenderingSystem;

    friend class ChaosEngine::SceneGraphSystem;

    entt::entity parent;
    Transform global; // To be updated from SceneGraph system according to transform tree
    bool needsGlobalRecalculation;
};

struct RenderComponent {
    RenderComponent(std::shared_ptr<Renderer::MaterialInstance> materialInstance,
                    std::shared_ptr<Renderer::RenderMesh> mesh)
            : materialInstance(std::move(materialInstance)), mesh(std::move(mesh)) {}

    // The Material Instance can't be stored here because depending on the used Graphics API the storage of a Material
    // Instance might change. So we need to store a pointer for now.
    std::shared_ptr<Renderer::MaterialInstance> materialInstance;
    std::shared_ptr<Renderer::RenderMesh> mesh;
};

struct CameraComponent {
    float fieldOfView = 45.0f;
    float near = 0.1f;
    float far = 100.0f;
    bool active = true;
    bool mainCamera = true;
};

struct NativeScriptComponent {
    friend class ChaosEngine::NativeScriptSystem;

    NativeScriptComponent(std::unique_ptr<ChaosEngine::NativeScript> &&script, bool active)
            : script(std::move(script)), active(active) {}

    ~NativeScriptComponent() = default;

    NativeScriptComponent(const NativeScriptComponent &o) = delete;

    NativeScriptComponent &operator=(const NativeScriptComponent &o) = delete;

    NativeScriptComponent(NativeScriptComponent &&o) noexcept
            : script(std::move(o.script)), active(o.active), initialized(o.initialized) {}

    NativeScriptComponent &operator=(NativeScriptComponent &&o) noexcept {
        if (this == &o)
            return *this;
        script = std::move(o.script);
        active = o.active;
        initialized = o.initialized;
        return *this;
    }

public:
    std::unique_ptr<ChaosEngine::NativeScript> script;
    bool active;
private:
    bool initialized = false;
};

struct UIComponent {
    bool active;
    glm::vec3 offsetPosition = glm::vec3(0);
    glm::vec3 offsetRotation = glm::vec3(0);
    glm::vec3 offsetScale = glm::vec3(1);
};

struct UITextComponent {
    std::shared_ptr<ChaosEngine::Font> font;
    ChaosEngine::FontStyle style;
    glm::vec4 textColor;
    std::string text;
};

struct UIRenderComponent {
    std::shared_ptr<Renderer::MaterialInstance> materialInstance;
    std::shared_ptr<Renderer::RenderMesh> mesh;
    glm::vec3 scaleOffset;
};

// Physics components ---------------------------------------------------------------------

struct StaticRigidBodyComponent {
    ChaosEngine::Physics2DBody body;
};

struct DynamicRigidBodyComponent {
    ChaosEngine::Physics2DBody body;
};

// Audio components ---------------------------------------------------------------------

struct AudioListenerComponent {
    bool active;
    glm::vec3 oldPosition; // Used for velocity calculation
};

struct AudioSourceComponent {
    ChaosEngine::AudioSource source;
    glm::vec3 oldPosition; // Used for velocity calculation
};
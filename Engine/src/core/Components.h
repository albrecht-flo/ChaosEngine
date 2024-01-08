#pragma once


#include <utility>
#include <memory>


#include "Engine/src/core/components/Transform.h"
using namespace ChaosEngine::Components;

#include "Engine/src/renderer/api/Material.h"
#include "Engine/src/renderer/api/RenderMesh.h"
#include "Engine/src/core/scriptSystem/NativeScript.h"
#include "Engine/src/core/assets/Font.h"
#include "Engine/src/core/physicsSystem/Physics2DBody.h"
#include "Engine/src/core/audioSystem/api/AudioSource.h"


namespace ChaosEngine {
    class NativeScriptSystem;
}

struct Meta {
    std::string name;
};


// ---------------------------------------------------------------------------------------------------------------------

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
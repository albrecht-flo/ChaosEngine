#include "Engine/src/renderer/api/Material.h"

#include "GraphicsContext.h"
#include "Engine/src/core/RenderingSystem.h"
#include "Engine/src/renderer/vulkan/api/VulkanMaterial.h"

using namespace Renderer;

// --------------------------------- Engine Base Materials -------------------------------------------------------------

std::vector<ShaderBindings> Material::StandardOpaqueSet0 = std::vector<ShaderBindings>(
        {ShaderBindings{.type = ShaderBindingType::UniformBuffer, .stage=ShaderStage::Vertex, .name="cameraUbo",
                .layout=std::make_optional(std::vector<ShaderBindingLayout>(
                        {
                                ShaderBindingLayout{.type = ShaderValueType::Mat4, .name ="view"},
                                ShaderBindingLayout{.type = ShaderValueType::Mat4, .name ="proj"}
                        }))
        }});

std::vector<ShaderPushConstantLayout> Material::StandardOpaquePushConstants = std::vector<ShaderPushConstantLayout>(
        {
                ShaderPushConstantLayout{.type = ShaderValueType::Mat4, .stage=ShaderStage::Vertex, .offset=0, .name ="modelMat"},
        });

// ------------------------------------ Class Members ------------------------------------------------------------------

MaterialRef Material::Create(const MaterialCreateInfo &info) {
    switch (GraphicsContext::currentAPI) {
        case GraphicsAPI::Vulkan:
            return MaterialRef(
                    VulkanMaterial::Create(RenderingSystem::GetContext(), RenderingSystem::GetCurrentRenderer(), info));
        default:
            assert("Invalid Graphics API" && false);
    }
    return MaterialRef(nullptr);
}

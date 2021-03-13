#include "Engine/src/renderer/api/Material.h"

#include "GraphicsContext.h"
#include "Engine/src/core/RenderingSystem.h"
#include "Engine/src/renderer/vulkan/api/VulkanMaterial.h"

using namespace Renderer;

std::vector<ShaderBindings> Material::StandardOpaqueSet0 = std::vector<ShaderBindings>(
        {ShaderBindings{.type = ShaderBindingType::UniformBuffer, .stage=ShaderStage::Vertex, .name="cameraUbo",
                .layout=std::vector<ShaderBindingLayout>(
                        {
                                ShaderBindingLayout{.type = ShaderValueType::Mat4, .name ="view"},
                                ShaderBindingLayout{.type = ShaderValueType::Mat4, .name ="proj"}
                        })
        }});

std::vector<ShaderPushConstantLayout> Material::StandardOpaquePushConstants = std::vector<ShaderPushConstantLayout>(
        {
                ShaderPushConstantLayout{.type = ShaderValueType::Mat4, .stage=ShaderStage::Vertex, .offset=0, .name ="modelMat"},
        });

std::unique_ptr<Material> Material::Create(const MaterialCreateInfo &info) {
    switch (GraphicsContext::currentAPI) {
        case GraphicsAPI::Vulkan:
            return std::make_unique<VulkanMaterial>(
                    RenderingSystem::GetContext(), RenderingSystem::GetCurrentRenderer(), info
            );
        case GraphicsAPI::None:
            assert("Invalid Graphics API" && false);
    }
    return nullptr;
}
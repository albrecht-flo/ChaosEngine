#include "EditorBaseAssets.h"

#include "core/utils/Logger.h"
#include "core/assets/ModelLoader.h"

#include "scripts/BaseMovementScript.h"

using namespace Editor;
using namespace Renderer;
using namespace ChaosEngine;

void EditorBaseAssets::loadBaseMeshes() {
    // Load meshes
    LOG_INFO("Creating quad buffers");
    auto quadAsset = ModelLoader::getQuad();
    auto vertexBuffer = Buffer::Create(quadAsset.vertices.data(), quadAsset.vertices.size() * sizeof(Vertex),
                                       BufferType::Vertex);
    auto indexBuffer = Buffer::Create(quadAsset.indices.data(), quadAsset.indices.size() * sizeof(uint32_t),
                                      BufferType::Index);
    quadROB = RenderMesh::Create(std::move(vertexBuffer), std::move(indexBuffer), quadAsset.indices.size());
    assetManager.registerMesh("Quad", quadROB, AssetManager::MeshInfo{});

    LOG_INFO("Creating hex buffers");
    auto hexAsset = ModelLoader::getHexagon();
    auto hexVertexBuffer = Buffer::Create(hexAsset.vertices.data(), hexAsset.vertices.size() * sizeof(Vertex),
                                          BufferType::Vertex);
    auto hexIndexBuffer = Buffer::Create(hexAsset.indices.data(), hexAsset.indices.size() * sizeof(uint32_t),
                                         BufferType::Index);
    hexROB = RenderMesh::Create(std::move(hexVertexBuffer), std::move(hexIndexBuffer), hexAsset.indices.size());
    assetManager.registerMesh("Hex", hexROB, AssetManager::MeshInfo{});

}

void EditorBaseAssets::loadBaseMaterials() {
    LOG_INFO("Creating materials");
    debugMaterial = Material::Create(MaterialCreateInfo{
            .stage = ShaderPassStage::Opaque,
            .fixedFunction = FixedFunctionConfiguration{.topology = Topology::TriangleList, .polygonMode = PolygonMode::Line,
                    .depthTest = true, .depthWrite = true},
            .vertexShader = "2DDebug",
            .fragmentShader = "2DStaticColoredSprite",
            .pushConstant = std::make_optional(Material::StandardOpaquePushConstants),
            .set0 = std::make_optional(Material::StandardOpaqueSet0),
            .set0ExpectedCount = Material::StandardOpaqueSet0ExpectedCount,
            .set1 = std::make_optional(std::vector<ShaderBindings>(
                    {ShaderBindings{.type = ShaderBindingType::UniformBuffer, .stage=ShaderStage::Fragment, .name="materialData",
                            .layout=std::make_optional(std::vector<ShaderBindingLayout>(
                                    {
                                            ShaderBindingLayout{.type = ShaderValueType::Vec4, .name ="color"},
                                    }))
                    }})),
            .set1ExpectedCount = 64,
            .name="DebugWireFrame",
    });
    assetManager.registerMaterial("DebugWireFrame", debugMaterial, AssetManager::MaterialInfo{.hasTintColor=true});

    texturedMaterial = Material::Create(MaterialCreateInfo{
            .stage = ShaderPassStage::Opaque,
            .fixedFunction = FixedFunctionConfiguration{.depthTest = true, .depthWrite = true},
            .vertexShader = "2DSprite",
            .fragmentShader = "2DStaticTexturedSprite",
            .pushConstant = std::make_optional(Material::StandardOpaquePushConstants),
            .set0 = std::make_optional(Material::StandardOpaqueSet0),
            .set0ExpectedCount = Material::StandardOpaqueSet0ExpectedCount,
            .set1 = std::make_optional(std::vector<ShaderBindings>(
                    {ShaderBindings{.type = ShaderBindingType::TextureSampler, .stage=ShaderStage::Fragment, .name="diffuseTexture"},
                     ShaderBindings{.type = ShaderBindingType::UniformBuffer, .stage=ShaderStage::Fragment, .name="materialData",
                             .layout=std::make_optional(std::vector<ShaderBindingLayout>(
                                     {
                                             ShaderBindingLayout{.type = ShaderValueType::Vec4, .name ="color"},
                                     }))
                     }
                    })),
            .set1ExpectedCount = 64,
            .name="TexturedSprite",
    });
    assetManager.registerMaterial("TexturedSprite", texturedMaterial, AssetManager::MaterialInfo{.hasTintColor=true});

}

void EditorBaseAssets::loadBaseTextures() {
    auto fallbackTexture1 = Texture::Create("TestAtlas.jpg");
    fallbackTexture = assetManager.registerTexture("TestAtlas.jpg", std::move(fallbackTexture1),
                                                   AssetManager::TextureInfo{});
}

void EditorBaseAssets::loadBaseScripts() {
    assetManager.registerNativeScript("BaseMovementScript",
                                      [](ChaosEngine::Entity e) {
                                          return std::unique_ptr<NativeScript>(new BaseMovementScript(e));
                                      },
                                      AssetManager::ScriptInfo{}
    );
}

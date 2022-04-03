#include "EditorBaseAssets.h"

#include "core/utils/Logger.h"
#include "core/assets/ModelLoader.h"

#include "scripts/BaseMovementScript.h"
#include "scripts/ButtonScript.h"

using namespace Editor;
using namespace Renderer;
using namespace ChaosEngine;

void EditorBaseAssets::loadBaseMeshes() {
    // Load meshes
    LOG_INFO("Creating quad buffers");
    auto quadAsset = ModelLoader::getQuad_PNCU();
    auto vertexBuffer = Buffer::Create(quadAsset.vertices.data(), quadAsset.vertices.size() * sizeof(VertexPNCU),
                                       BufferType::Vertex);
    auto indexBuffer = Buffer::Create(quadAsset.indices.data(), quadAsset.indices.size() * sizeof(uint32_t),
                                      BufferType::Index);
    quadROB = RenderMesh::Create(std::move(vertexBuffer), std::move(indexBuffer), quadAsset.indices.size());
    assetManager.registerMesh("Quad", quadROB, AssetManager::MeshInfo{});

    LOG_INFO("Creating hex buffers");
    auto hexAsset = ModelLoader::getHexagon();
    auto hexVertexBuffer = Buffer::Create(hexAsset.vertices.data(), hexAsset.vertices.size() * sizeof(VertexPNCU),
                                          BufferType::Vertex);
    auto hexIndexBuffer = Buffer::Create(hexAsset.indices.data(), hexAsset.indices.size() * sizeof(uint32_t),
                                         BufferType::Index);
    hexROB = RenderMesh::Create(std::move(hexVertexBuffer), std::move(hexIndexBuffer), hexAsset.indices.size());
    assetManager.registerMesh("Hex", hexROB, AssetManager::MeshInfo{});


    LOG_INFO("Creating UI quad buffers");
    auto uiQuadAsset = ModelLoader::getQuad_PCU();
    auto uiQuadVertexBuffer = Buffer::Create(uiQuadAsset.vertices.data(),
                                             uiQuadAsset.vertices.size() * sizeof(VertexPCU), BufferType::Vertex);
    auto uiQuadIndexBuffer = Buffer::Create(uiQuadAsset.indices.data(), uiQuadAsset.indices.size() * sizeof(uint32_t),
                                            BufferType::Index);
    quadROB = RenderMesh::Create(std::move(uiQuadVertexBuffer), std::move(uiQuadIndexBuffer),
                                 uiQuadAsset.indices.size());
    assetManager.registerMesh("UI/Quad", quadROB, AssetManager::MeshInfo{});

}

void EditorBaseAssets::loadBaseMaterials() {
    const auto BaseVertexLayout = VertexLayout{.binding = 0, .stride = sizeof(VertexPNCU), .inputRate=InputRate::Vertex,
            .attributes = std::vector<VertexAttribute>(
                    {
                            VertexAttribute{0, VertexFormat::RGB_FLOAT, offsetof(VertexPNCU, pos)},
                            VertexAttribute{1, VertexFormat::RGB_FLOAT, offsetof(VertexPNCU, color)},
                            VertexAttribute{2, VertexFormat::RGB_FLOAT, offsetof(VertexPNCU, normal)},
                            VertexAttribute{3, VertexFormat::RG_FLOAT, offsetof(VertexPNCU, uv)},
                    })};

    LOG_INFO("Creating materials");
    debugMaterial = Material::Create(MaterialCreateInfo{
            .stage = ShaderPassStage::Opaque,
            .vertexLayout = BaseVertexLayout,
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
            .vertexLayout = BaseVertexLayout,
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

    auto uiMaterial = Material::Create(MaterialCreateInfo{
            .stage = ShaderPassStage::Opaque,
            .vertexLayout = VertexLayout{.binding = 0, .stride = sizeof(VertexPCU), .inputRate=InputRate::Vertex,
                    .attributes = std::vector<VertexAttribute>(
                            {
                                    VertexAttribute{0, VertexFormat::RGB_FLOAT, offsetof(VertexPCU, pos)},
                                    VertexAttribute{1, VertexFormat::RGBA_FLOAT, offsetof(VertexPCU, color)},
                                    VertexAttribute{2, VertexFormat::RG_FLOAT, offsetof(VertexPCU, uv)},
                            })},
            .fixedFunction = FixedFunctionConfiguration{.depthTest = true, .depthWrite = true},
            .vertexShader = "UIBase",
            .fragmentShader = "UI",
            .pushConstant = std::make_optional(Material::StandardOpaquePushConstants),
            .set0 = std::make_optional(Material::StandardOpaqueSet0),
            .set0ExpectedCount = Material::StandardOpaqueSet0ExpectedCount,
            .set1 = std::make_optional(std::vector<ShaderBindings>(
                    {
                            ShaderBindings{.type = ShaderBindingType::TextureSampler, .stage=ShaderStage::Fragment, .name="texture"},
                            ShaderBindings{.type = ShaderBindingType::UniformBuffer, .stage=ShaderStage::Fragment, .name="materialData",
                                    .layout=std::make_optional(std::vector<ShaderBindingLayout>(
                                            {
                                                    ShaderBindingLayout{.type = ShaderValueType::Vec4, .name ="color"},
                                            }))
                            }
                    })),
            .set1ExpectedCount = 64,
            .name="UIMaterial",
    });
    assetManager.registerMaterial("UI", uiMaterial, AssetManager::MaterialInfo{.hasTintColor=true});


}

void EditorBaseAssets::loadBaseTextures() {
    auto fallbackTexture1 = Texture::Create("TestAtlas.jpg");
    fallbackTexture = assetManager.registerTexture("TestAtlas.jpg", std::move(fallbackTexture1),
                                                   AssetManager::TextureInfo{});

    auto borderTexture = Texture::Create("Border_128.png");
    assetManager.registerTexture("UI/Border", std::move(borderTexture), AssetManager::TextureInfo{});
}

void EditorBaseAssets::loadBaseScripts() {
    assetManager.registerNativeScript("BaseMovementScript",
                                      [](ChaosEngine::Entity e) {
                                          return std::unique_ptr<NativeScript>(new BaseMovementScript(e));
                                      },
                                      AssetManager::ScriptInfo{}
    );
    assetManager.registerNativeScript("UI/ButtonScript",
                                      [](ChaosEngine::Entity e) {
                                          return std::unique_ptr<NativeScript>(new ButtonScript<true>(e));
                                      },
                                      AssetManager::ScriptInfo{}
    );
}

void EditorBaseAssets::loadBaseFonts() {
    assetManager.loadFont("OpenSauceSans", "fonts/OpenSauceSans-Regular.ttf", FontStyle::Regular, 16.0f);
    assetManager.loadFont("OpenSauceSans", "fonts/OpenSauceSans-Italic.ttf", FontStyle::Italic, 16.0f);
    assetManager.loadFont("OpenSauceSans", "fonts/OpenSauceSans-Bold.ttf", FontStyle::Bold, 16.0f);
}

#include "CustomImGui.h"

#include "ChaosEngine.h"
#include "Engine/src/renderer/vulkan/image/VulkanTexture.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

#include <vector>
#include <string>

CustomImGui::CustomImGuiState CustomImGui::state{};

// Source: https://gist.github.com/Pikachuxxxx/a3796bb193ca0aaed4ad4f591b2dab07
void CustomImGui::ImGuiEnableDocking(const std::function<void(void)> &menuCallback) {
    static bool opt_fullscreen = true;
    static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
        dockspaceFlags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);
    } else {
        Logger::E("ImGui", "Cannot Dock Windows");
    }

    menuCallback();

    ImGui::End();
}

void CustomImGui::RenderLogWindow(const std::string &title) {
    ImGui::Begin(title.c_str());
    if (ImGui::Button("Toggle auto scroll"))
        state.followLog = !state.followLog;
    std::vector<std::string> logger = Logger::GetLogBuffer();

    ImGui::Separator();
    if (ImGui::BeginListBox("##LogList", ImVec2{-FLT_MIN, -FLT_MIN})) {
        for (const auto &str: logger) {
            ImGui::TextUnformatted(str.c_str(), &str.back());
        }

        auto left_up = ImGui::GetWindowPos();
        auto right_down = ImVec2{left_up.x + ImGui::GetWindowWidth(), left_up.y + ImGui::GetWindowHeight()};
        if (ImGui::IsMouseHoveringRect(left_up, right_down) && ImGui::GetIO().MouseWheel != 0)
            state.followLog = false;
        if (ImGui::IsMouseHoveringRect(left_up, right_down) && ImGui::GetIO().MouseWheel < 0 &&
            ImGui::GetScrollY() == ImGui::GetScrollMaxY())
            state.followLog = true;

        if (state.followLog)
            ImGui::SetScrollY(ImGui::GetScrollMaxY());
        ImGui::EndListBox();
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed to create log list!");
    }
    ImGui::End();
}


ImVec2 CustomImGui::RenderSceneViewport(const Renderer::Framebuffer &framebuffer, const std::string &title) {
    using namespace Renderer;
    if (state.sceneImageGPUHandles.empty()) {
        uint32_t sceneImageGPUHandleCount;
        switch (GraphicsContext::currentAPI) {
            case Renderer::GraphicsAPI::Vulkan:
                // TODO: Handle imgui textures properly
                sceneImageGPUHandleCount = dynamic_cast<const VulkanContext &>(RenderingSystem::GetContext())
                                                   .getSwapChain().size() + 1; // # swapchain images in flight
                break;
            default:
                sceneImageGPUHandleCount = 1;
                break;
        }
        state.sceneImageGPUHandles = std::vector<void *>(sceneImageGPUHandleCount, nullptr);
    }

    // Get attachment texture
    switch (GraphicsContext::currentAPI) {
        case GraphicsAPI::Vulkan: {
            const auto &tex = dynamic_cast<const VulkanTexture &>(framebuffer.getAttachmentTexture(
                    Renderer::AttachmentType::Color, 0));
            if (state.sceneImageGPUHandles[state.currentFrame] == nullptr) {
                state.sceneImageGPUHandles[state.currentFrame] = ImGui_ImplVulkan_AddTexture(tex.getSampler(),
                                                                                             tex.getImageView(),
                                                                                             tex.getImageLayout());
            } else {
                ImGui_ImplVulkan_UpdateTexture((VkDescriptorSet) state.sceneImageGPUHandles[state.currentFrame],
                                               tex.getSampler(), tex.getImageView(), tex.getImageLayout());
            }
            break;
        }
        default: {
            Logger::E("RenderSceneViewport", "Unsupported Graphics API!");
            return ImVec2{0.0f, 0.0f};
        }
    }

    // Render viewport
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin(title.c_str());
    // Size and Resize handling
    auto size = ImGui::GetContentRegionAvail();
    if (state.previousSize.x != size.x || state.previousSize.y != size.y) {
        state.previousSize = size;
        RenderingSystem::GetCurrentRenderer().requestViewportResize(glm::vec2(size.x, size.y));
    }

    ImGui::Image(state.sceneImageGPUHandles[state.currentFrame],
                 ImVec2(static_cast<float>(framebuffer.getWidth()), static_cast<float>(framebuffer.getHeight())));

    ImGui::End();
    ImGui::PopStyleVar();

    state.currentFrame = (state.currentFrame < state.sceneImageGPUHandles.size() - 1) ? state.currentFrame + 1 : 0;
    return state.previousSize;
}

// ------------------------------------ Trees --------------------------------------------------------------------------

static const ImGuiTreeNodeFlags node_flags_base = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
static const ImGuiTreeNodeFlags node_flags_leaf =
        node_flags_base | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
static const ImGuiTreeNodeFlags node_flags_selected = ImGuiTreeNodeFlags_Selected;

// See ImGui Issue: https://github.com/ocornut/imgui/issues/581
// And demo: https://github.com/ocornut/imgui/commit/ac501102fc7524433c2343f2fa2cc47ea08f548e
bool CustomImGui::TreeLeaf(const char *label, const uint32_t id, uint32_t *id_ptr) {
    bool nodeState = ImGui::TreeNodeEx(label, node_flags_leaf | (*id_ptr == id ? node_flags_selected : 0));
    if (ImGui::IsItemClicked() && *id_ptr != id) {
        LOG_DEBUG("Updating id to {}", id);
        *id_ptr = id;
    }
    return nodeState;
}

bool CustomImGui::TreeNodeBegin(const char *label, const uint32_t id, uint32_t *id_ptr) {
    bool nodeState = ImGui::TreeNodeEx(label, node_flags_base | (*id_ptr == id ? node_flags_selected : 0));
    if (ImGui::IsItemClicked() && *id_ptr != id) {
        LOG_DEBUG("Updating id to {}", id);
        *id_ptr = id;
    }
    return nodeState;
}

void CustomImGui::TreeNodeEnd() {
    ImGui::TreePop();
}

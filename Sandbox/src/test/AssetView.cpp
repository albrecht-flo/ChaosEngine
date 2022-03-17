#include "AssetView.h"
#include "Engine/src/core/Utils/Logger.h"
#include "CustomImGui.h"

#include <imgui.h>
#include <filesystem>

namespace fs = std::filesystem;

using namespace CustomImGui;

AssetView::AssetViewState AssetView::state = {};

bool AssetView::renderAssetView() {
    bool open = ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (open) {
        if (ImGui::BeginTable("table_nested1", 2,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoClip)) {

            ImGui::TableNextColumn();
            ImGui::BeginChild("Tree panel");
            if (CoreImGui::TreeNodeBegin(1u, &state.selected, "Node 1")) {
                CoreImGui::TreeLeaf(2u, &state.selected, "Leaf 1");
                CoreImGui::TreeLeaf(3u, &state.selected, "Leaf 2");
                CoreImGui::TreeLeaf(4u, &state.selected, "Leaf 3");
                CoreImGui::TreeNodeEnd();
            }
            for (uint32_t i = 5; i < 50; ++i) {
                CoreImGui::TreeLeaf(i, &state.selected, "Node %u", i);
            }
            ImGui::EndChild();

            ImGui::TableNextColumn();
            ImGui::BeginChild("Assets panel");
            ImGui::Text("Assets");
            fs::path assetPath("./Assets");
            if (!fs::exists(assetPath)) {
                create_directory(assetPath);
                LOG_INFO("Created Directory {0}", fs::absolute(assetPath).string().c_str());
            }
//            ImGui::Text("exists() = %s", fs::exists(assetPath) ? "true" : "false");
//            ImGui::Text("dir() = %s", fs::is_directory(assetPath) ? "true" : "false");
//            ImGui::Text("root_name() = %s", assetPath.root_name().c_str());
//            ImGui::Text("root_path() = %s", assetPath.root_path().c_str());
//            ImGui::Text("relative_path() = %s", assetPath.relative_path().c_str());
//            ImGui::Text("parent_path() = %s", assetPath.parent_path().c_str());
//            ImGui::Text("filename() = %s", assetPath.filename().c_str());
//            ImGui::Text("stem() = %s", assetPath.stem().c_str());
//            ImGui::Text("extension() = %s", assetPath.extension().c_str());
            for (const auto &entry: fs::directory_iterator(assetPath)) {
                ImGui::Text("%s%s", entry.path().string().c_str(), (entry.is_directory() ? "/" : ""));
            }

            ImGui::EndChild();

            ImGui::EndTable();
        }
    }
    ImGui::End();
    return open;
}

#include "AssetView.h"
#include <imgui.h>
#include "CustomImGui.h"

using namespace CustomImGui;

AssetView::AssetViewState AssetView::state = {};

bool AssetView::renderAssetView() {
    bool open = ImGui::Begin("Assets");
    if (open) {
        if (ImGui::BeginTable("table_nested1", 2,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {

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
            ImGui::Text("Here will be the assets");
            ImGui::EndChild();

            ImGui::EndTable();
        }
    }
    ImGui::End();
    return open;
}

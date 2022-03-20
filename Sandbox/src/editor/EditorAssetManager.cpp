#include "EditorAssetManager.h"

#include <filesystem>

#include "Engine/src/core/Utils/Logger.h"

#include <imgui.h>
#include <nfd.h>

using namespace Editor;
namespace fs = std::filesystem;

static void openFileDialog() {
    fs::path assetPath("./");
    const auto basePath = fs::absolute(assetPath).string();
    LOG_INFO("Base Directory {}", basePath.c_str());

    nfdchar_t *outPath;
    nfdfilteritem_t filterItem[1] = {{"Image", "png,jpg"}};
    nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, basePath.c_str());
    if (result == NFD_OKAY) {
        LOG_DEBUG("Success!");
        LOG_DEBUG("Result: {}", outPath);
        NFD_FreePath(outPath);
    } else if (result == NFD_CANCEL) {
        LOG_DEBUG("User pressed cancel.");
    } else {
        LOG_DEBUG("Error: %s", NFD_GetError());
    }

}

void EditorAssetManager::renderAssetMenu() {
    if (ImGui::BeginMenu("Assets")) {
        if (ImGui::MenuItem("Import")) {
            openFileDialog();
        }
        ImGui::EndMenu();
    }
}

void EditorAssetManager::createProject() {

}

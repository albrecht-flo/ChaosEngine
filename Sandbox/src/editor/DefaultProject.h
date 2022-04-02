#pragma once

#include "Engine/src/core/assets/AssetManager.h"
#include "Engine/src/core/Scene.h"
#include "EditorBaseAssets.h"

namespace Editor {

    void loadDefaultSceneEntities(ChaosEngine::Scene &scene, EditorBaseAssets &assets,
                                  ChaosEngine::AssetManager &assetManager);

}

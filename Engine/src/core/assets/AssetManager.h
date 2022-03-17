#pragma once
#include <string>

class AssetManager {
    enum class ResourceType {
        Scene, Entity, Mesh, Material, Texture, Script
    };
    class ResourceHandle {};
public:
    // TODO: COnstruction

    /// Load resource from file into the engine
    ResourceHandle loadResource(ResourceType type, const std::string& uri);

    /// Destroy resource from the engine
    void destroyResource(ResourceHandle handle);

    /// Retrieve loaded resource by UIR
    ResourceHandle getResource(ResourceType type, const std::string& uri) const;


private:
    std::string baseResourcePath;
};




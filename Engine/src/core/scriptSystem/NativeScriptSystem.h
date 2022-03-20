#pragma once

namespace ChaosEngine {
    class ECS;

    class NativeScriptSystem {
    public:
        NativeScriptSystem() = default;

        ~NativeScriptSystem() = default;

        NativeScriptSystem(const NativeScriptSystem &o) = delete;

        NativeScriptSystem &operator=(const NativeScriptSystem &o) = delete;

        NativeScriptSystem(NativeScriptSystem &&o) = delete;

        NativeScriptSystem &operator=(NativeScriptSystem &&o) = delete;

        void init(ECS& ecs);

        void update(ECS& ecs, float deltaTime);

    private:
    };

}

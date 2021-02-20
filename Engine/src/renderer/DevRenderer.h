#pragma once

#include <Engine/src/renderer/api/GraphicsContext.h>

namespace Renderer {

    /**
     * This Class contains a development render to test and develop new features and interfaces. <br/>
     *
     * <b>NOT to be used in production</b>
     */
    class DevRenderer {
    private:
    public:
        DevRenderer(std::unique_ptr<GraphicsContext> &&context);

    public:
        ~DevRenderer() = default;

        static DevRenderer Create(Window &window, GraphicsAPI api);

        /// Loads all resources to put this renderer into working mode
        void init();

    private:
        std::unique_ptr<GraphicsContext> context;

    };

}

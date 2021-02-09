#pragma once

#include <src/renderer/api/GraphicsContext.h>

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

    private:
        std::unique_ptr<GraphicsContext> context;

    };

}

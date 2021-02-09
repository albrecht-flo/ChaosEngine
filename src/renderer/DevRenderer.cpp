#include "DevRenderer.h"

namespace Renderer {

    DevRenderer::DevRenderer(std::unique_ptr<GraphicsContext> &&context)
            : context(std::move(context)) {}

    DevRenderer DevRenderer::Create(Window &window, GraphicsAPI api) {
        auto context = GraphicsContext::Create(window, api);

        return DevRenderer(std::move(context));
    }
}
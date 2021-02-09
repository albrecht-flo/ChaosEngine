#pragma once

#include "src/renderer/window/Window.h"
#include <memory>

namespace Renderer {

    /// Through this enum the used Graphics API can be defined.
    enum class GraphicsAPI {
        Vulkan, OpenGl
    };

    /** This is a wrapper class for different graphics context objects that can be used to be passed down for further
     *  Object creation.
     */
    class GraphicsContext {
    public:
        /** Creates and <b>initializes</b> the context corresponding to the passed <i>api<i/>.
         *
         * @param window The window object the created graphics context is bound to
         * @param api Graphics API (atm only Vulkan)
         * @return Pointer to the created context
         */
        static std::unique_ptr<GraphicsContext> Create(Window &window, GraphicsAPI api);

        //TODO: Swap buffers here or in Renderer?
    };

}

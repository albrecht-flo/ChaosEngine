#pragma once

#include "Engine/src/renderer/window/Window.h"
#include "BufferedGPUResource.h"
#include <memory>

namespace Renderer {

    /// Through this enum the used Graphics API can be defined.
    enum class GraphicsAPI {
        None, Vulkan
    };

    /** This is a wrapper class for different graphics context objects that can be used to be passed down for further
     *  Object creation. <br/>
     *
     *  Their task is to create a graphics context in which further objects such as VertexArrays, Textures, Shaders etc.
     *  can be created and used. <br/>
     *
     *  As part of this it also has to handle showing the next image on the presentation surface and ensuring that
     *  surfaces validity. (eg. SwapChain recreation)
     */
    class GraphicsContext {
    public:
        virtual ~GraphicsContext() = default;

        /** Creates and <b>initializes</b> the context corresponding to the passed <i>api<i/>.
         *
         * @param window The window object the created graphics context is bound to
         * @param api Graphics API (atm only Vulkan)
         * @return Pointer to the created context
         */
        static std::unique_ptr<GraphicsContext> Create(Window &window, GraphicsAPI api);

        /**
         * This method submits all recorded draw commands to the GPU and starts rendering the next Frame.
         * @return <i>false</i> if the presenting surface has changed and was recreated.
         */
        virtual bool flushCommands() = 0;

        /**
         * Deletes the passed in resource when all frames in which it could have been used have passed.
         * @param resource The resource to be destroyed when it is no longer in use
         */
        virtual void destroyBuffered(std::unique_ptr<BufferedGPUResource> resource) = 0;

        /**
         * This method handles per frame tasks of the context such as destruction of buffered resources.
         */
        virtual void tickFrame() = 0;

        static GraphicsAPI currentAPI;
    };

}

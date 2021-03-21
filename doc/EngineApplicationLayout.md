# Engine Engine Layout
## Engine (Main-Loop)
1. create context
2. load scene
    - Apply static transforms
    - Create all resources
3. start Game Loop

## Scene
- SceneConfiguration:   
    - Renderer
    - Renderer settings
    - Other scene wide settings
- Scene Graph
  - Entities
## Layer


## GPU-Shared resource management
### Requirements
- Can be in-use on the GPU while already destroyed in scene
    - At least the current frame
    - At most the number of buffered frames (if all frames have been prepared)
- Can be shared between entities
    - Only when the last instance of the resource is deleted the gpu resource can be scheduled for deletion
    
### Idea
- Destructor moves the resource to a destruction queue (no resources are freed)
- During rendering system apply changes
    - Before all other changes
    - *In GraphicsContext* implementation   
    - Pop from front of queue `while(res->frame == currentframe - bufferFrameCount)`
        - if currentFrame, frame and bufferFrame count are unsigned this will work
            - `0 - x = 256 - x`
    


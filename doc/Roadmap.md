# Roadmap for this Engine

### Current: ECS

- Refactor Rendering to be an ECS system
    - Collect issues that need refactoring
        - Refactor storage management of Rendering system
        - Refactor vulkan objects
            - Vulkan Context object for objects every application needs (instance, devices, swapchain)

### Next

- Move to Engine structure
    - Compile Engine separate of application in separate src dirs
    - Define clear public API
    - Introduce Scenes and layers

_______________________________________________________________________________

## Future
### Frame setup to minimize synchronization
Idea: Group the tasks into Phases:
0. Join waiting Async tasks (Need to supply a dependecy list)
    * Start all non dependent apply jobs
1. Run Stuff like the input system that are required by all systems while (2.)
2. Start all apply jobs (multi threaded as they only apply changes for one Component each) 
3. Run Engine Systems in parallel
    - Can't interfere with each other as all read constant data
    - Changes get cached so only the change*() methods need synchronization


- Cache changes (synchronize the change methods for multithreading).
- Apply these changes on a per component type basis (Prevents cross access)
- Now the Systems can read from the component data without sync
- Data changes can be cached while other systems read the components and process them
- Asynchronous Processes can join the loop during the apply phase
- Systems can be **ordered** to ensure minimal synchronization
    - Input System handles its stuff
    - Physics System **applies Transform changes**
    - Rendering System **applies RenderComponent changes**
    - Audio System **applies AudioComponent changes**  
    ______ __Now the Systems can read and modify the components in parallel__ ______
    - *Now systems can read their components without collisions*  
      *and the data can be modified as all components cache their changes*
    - Rendering System **reads Transforms** and **render components**
    - Audio System **reads Transforms** and **audio components**
    - Scripts run and modify components (The modify methods need sync)
    - Particle System runs and modifies components
- Using component dependencies the systems can even start to run as soon as 
  changes to their required components have been applied
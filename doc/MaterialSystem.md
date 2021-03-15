# Material System

## Design

### Aspects

- HW API -> Vulkan, DX etc.
- components
    - Attachments
    - Fixed function
    - Vertex Input
    - Shader Sources
    - Descriptors (UniformBuffer, Samplers, PushConstants/ConstantBuffer)

#### Grouping
These objects have a downward compositional relationship.
- RenderPass:
    - Attachments
- ShaderPass:
    - Vertex Input
    - Fixed Function
- Material:
    - Shader Sources
    - Descriptor Layout
- MaterialInstance:
    - Resource collection
    - Can create a Descriptors(-Sets)

### Responsibilities
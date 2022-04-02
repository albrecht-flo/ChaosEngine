# Material System

## Design

### Aspects

- HW API -> Vulkan, DX etc.
- components
    - Attachments
    - Fixed function
    - VertexPNCU Input
    - Shader Sources
    - Descriptors (UniformBuffer, Samplers, PushConstants/ConstantBuffer)

#### Grouping
These objects have a downward compositional relationship.
- RenderPass:
    - Attachments
- ShaderPass:
    - VertexPNCU Input
    - Fixed Function
- Material:
    - Shader Sources
    - Descriptor Layout
- MaterialInstance:
    - Resource collection
    - Can create a Descriptors(-Sets)

### Responsibilities
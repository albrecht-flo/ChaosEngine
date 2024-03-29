#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec4 in_Color;

layout(location = 0) out vec4 out_fragColor;

// Set 0 defines per frame data
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} cameraUbo;

void main() {
    out_fragColor = in_Color;
    gl_Position = cameraUbo.proj * cameraUbo.view * vec4(in_Position, 1.0);
}

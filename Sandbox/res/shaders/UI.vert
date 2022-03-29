#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec2 in_UVs;

layout(location = 0) out vec4 out_fragColor;
layout(location = 1) out vec2 out_fragUVs;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} cameraUbo;

layout(push_constant) uniform ModelData {
    layout(offset = 0) mat4 modelMat;
} modelData;

void main() {
    out_fragColor = in_Color;
    // We only want the rotation effect of the model matrix, scale can be ignored
    out_fragUVs = in_UVs;
    vec4 position = modelData.modelMat * vec4(in_Position, 1.0);
    gl_Position = cameraUbo.proj * cameraUbo.view * position;
}

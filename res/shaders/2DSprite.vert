#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Color;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec2 in_UVs;

layout(location = 0) out vec3 out_fragColor;
layout(location = 1) out vec3 out_fragNormal;
layout(location = 2) out vec2 out_fragUVs;
layout(location = 3) out vec3 out_fragWorldPos;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} cameraUbo;

layout(push_constant) uniform ModelData {
    layout(offset = 0) mat4 model;
} modelData;

void main() {
    out_fragColor = in_Color;
    // We only want the rotation effect of the model matrix, scale can be ignored
    out_fragNormal = normalize(mat3(transpose(inverse(modelData.model))) * in_Normal);
    out_fragUVs = in_UVs;
    vec4 position = modelData.model * vec4(in_Position, 1.0);
    out_fragWorldPos = position.xyz;
    gl_Position = cameraUbo.proj * cameraUbo.view * position;
}

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 worldLightPosition;
    vec4 worldLightColor;
} ubo;

layout(push_constant) uniform ModelData {
    mat4 model;
} modelData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUVs;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUVs;
layout(location = 3) out vec3 fragWorldPos;

void main() {
    fragColor = inColor;
    // We only want the rotation effect of the model matrix, scale can be ignored
    fragNormal = normalize(mat3(transpose(inverse(modelData.model))) * inNormal);
    fragUVs = inUVs;
    vec4 position = modelData.model * vec4(inPosition, 1.0);
    fragWorldPos = position.xyz;
    gl_Position = ubo.proj * ubo.view * position;
}

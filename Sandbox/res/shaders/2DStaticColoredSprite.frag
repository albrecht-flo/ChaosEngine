#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input from vertex shader
layout(location = 0) in vec3 in_fragColor;// Interpolated per vertex color
layout(location = 1) in vec3 in_fragNormal;// Interpolated normal vector
layout(location = 2) in vec2 in_fragUVs;// Interpolated texture coordinate
layout(location = 3) in vec3 in_fragWorldPos;// Interpolated world position of fragment
// Output to framebuffer attachment
layout(location = 0) out vec4 out_Color;

layout(set = 1, binding = 0) uniform FramentData {
    vec4 color;
} materialData;

void main() {
    out_Color = materialData.color;
}
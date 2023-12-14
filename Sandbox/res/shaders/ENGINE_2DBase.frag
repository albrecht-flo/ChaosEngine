#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input from vertex shader
layout(location = 0) in vec3 in_fragColor;// Interpolated per vertex color
layout(location = 1) in vec3 in_fragNormal;// Interpolated normal vector
layout(location = 2) in vec2 in_fragUVs;// Interpolated texture coordinate
layout(location = 3) in vec3 in_fragWorldPos;// Interpolated world position of fragment
// Output to framebuffer attachment
layout(location = 0) out vec4 out_Color;

// Set 1 can be use freely for material data
// Set 2 undefined
// Set 3 undefined

void main() {
    out_Color = vec4(1, 0.3, 1, 1);
}
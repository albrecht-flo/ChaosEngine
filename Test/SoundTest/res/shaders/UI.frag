#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input from vertex shader
layout(location = 0) in vec4 in_fragColor;// Interpolated per vertex color
layout(location = 1) in vec2 in_fragUVs;// Interpolated texture coordinate
// Output to framebuffer attachment
layout(location = 0) out vec4 out_Color;

// Material Parameters
layout(set = 1, binding = 0) uniform sampler2D diffuseTexture;// Model texture
layout(set = 1, binding = 1) uniform FramentData {
    vec4 color;
} materialData;

void main() {
    // Get the base color of the fragment
    vec4 color = texture(diffuseTexture, in_fragUVs);
    // Combine calculated texture color with fragment color // used for tinting models
    out_Color = in_fragColor * color * materialData.color;
}
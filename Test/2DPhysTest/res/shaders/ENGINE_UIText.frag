#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input from vertex shader
layout(location = 0) in vec4 in_fragColor;// Interpolated per vertex color
layout(location = 1) in vec2 in_fragUVs;// Interpolated texture coordinate
// Output to framebuffer attachment
layout(location = 0) out vec4 out_Color;

// Material Parameters
layout(set = 1, binding = 0) uniform sampler2D diffuseTexture;// Model texture

void main() {
    // Get the base color of the fragment
    vec4 color = texture(diffuseTexture, in_fragUVs);
    if (color.r == 0){
        discard;
    }
    // Combine calculated texture color with fragment color // used for tinting models
    float text = color.r;
    out_Color =  vec4(in_fragColor.rgb, in_fragColor.a * text);
}
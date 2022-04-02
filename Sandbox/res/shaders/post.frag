#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUVs;
// Output
layout(location = 0) out vec4 outColor;
// Uniforms for textures
layout(set = 0, binding = 0) uniform sampler2D mainSceneTex;// Main scene framebuffer image
layout(set = 0, binding = 1) uniform sampler2D depthBufferTex;// Depth buffer image
layout(set = 0, binding = 2) uniform sampler2D backgroundTex;// Background image
layout(set = 0, binding = 3) uniform sampler2D imGuiTex;// ImGui framebuffer image
// Uniforms of camera settings
layout(push_constant) uniform CameraParams {
    float near;
    float far;
} camera;

// Modes of post processing shader
#define NONE 0
#define BLACKNWHITE 1
#define COLOR_HALF_DEPTH 2
#define COLOR_BACKGROUND 3
#define IMGUI_ONLY 4
#define COLOR_BACKGROUND_IMGUI 5

#define EFFECT COLOR_BACKGROUND_IMGUI

// From https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/shadowmapping/quad.frag
float LinearizeDepth(float depth) {
    float n = camera.near;// camera z near // ! NEEDS TO BE LIKE PROJECTION MATRIX
    float f = camera.far;// camera z far // ! NEEDS TO BE LIKE PROJECTION MATRIX
    float z = depth;
    return (2.0 * n) / (f + n - z * (f - n));
}

/*
    This shader mixes the framebuffer images of previous render passes together.
*/
void main() {

    // Just the main scene framebuffer
    if (EFFECT == 0) {
        outColor = texture(mainSceneTex, fragUVs);
    }
    // Main scene in black and white
    else if (EFFECT == BLACKNWHITE) {
        vec4 color = texture(mainSceneTex, fragUVs);
        float bwColor = (color.x + color.y + color.z)/3.0;
        outColor = vec4(bwColor, bwColor, bwColor, color.a);
    }
    // Left half color buffer, right half depth buffer
    else if (EFFECT == COLOR_HALF_DEPTH) {
        vec4 color;
        if (fragUVs.x < 0.5)
        color = texture(mainSceneTex, fragUVs);
        else {
            float depth = LinearizeDepth(texture(depthBufferTex, fragUVs).r);
            color = vec4(depth, depth, depth, 1.0);
        }
        outColor = color;
    }
    // Main scene with background image, no transparency
    else if (EFFECT == COLOR_BACKGROUND) {
        vec4 color;
        float depth = texture(depthBufferTex, fragUVs).r;
        if (depth == 1.0)
        color = texture(backgroundTex, fragUVs);
        else
        color = texture(mainSceneTex, fragUVs);
        outColor = color;
    }
    // Only ImGui framebuffer
    else if (EFFECT == IMGUI_ONLY) {
        outColor = texture(imGuiTex, fragUVs);
    }
    // Main scene framebuffer with backgorund image and ImGui overlay
    else if (EFFECT == COLOR_BACKGROUND_IMGUI) {
        vec4 color;

        float depth = texture(depthBufferTex, fragUVs).r;
        if (depth == 1.0)
        color = texture(backgroundTex, fragUVs);
        else
        color = texture(mainSceneTex, fragUVs);

        vec4 guiFragment = texture(imGuiTex, fragUVs);

        if (guiFragment.a != 1)
        color = vec4(mix(color.rgb, guiFragment.rgb, 0.8), 1.0);

        outColor = color;
    }
}
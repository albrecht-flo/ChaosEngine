#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUVs;
// Output
layout(location = 0) out vec4 outColor;

// Configuration data
layout(set = 0, binding = 0) uniform Configuration {
    float cameraNear;
    float cameraFar;
} config;

// Uniforms for textures
layout(set = 0, binding = 1) uniform sampler2D mainSceneTex;// Main scene framebuffer image
layout(set = 0, binding = 2) uniform sampler2D depthBufferTex;// Depth buffer image
layout(set = 0, binding = 3) uniform sampler2D uiBufferTex;// UI frambuffer image
layout(set = 0, binding = 4) uniform sampler2D textBufferTex;// Text frambuffer image

// From https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/shadowmapping/quad.frag
float LinearizeDepth(float depth) {
    float n = config.cameraNear;// camera z near // ! NEEDS TO BE LIKE PROJECTION MATRIX
    float f = config.cameraFar;// camera z far // ! NEEDS TO BE LIKE PROJECTION MATRIX
    float z = depth;
    return (2.0 * n) / (f + n - z * (f - n));
}
/*
    This shader mixes the framebuffer images of previous render passes together.
*/
void main() {
    // Scene
    vec4 color = texture(mainSceneTex, fragUVs);

    // Overlay UI
    vec4 uiColor = texture(uiBufferTex, fragUVs);
    // Overlay Text
    vec4 textColor = texture(textBufferTex, fragUVs);
    vec3 finalColor = mix(color.rgb, uiColor.rgb, uiColor.a);
    finalColor = mix(finalColor, textColor.rgb, textColor.a);
    outColor = vec4(finalColor, color.a);
}
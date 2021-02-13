#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUVs;
// Output
layout(location = 0) out vec4 outColor;
// Uniforms for textures
layout(set = 0, binding = 0) uniform Configuration { // Configuration data
    float cameraNear;
    float cameraFar;
} config;

layout(set = 0, binding = 1) uniform sampler2D mainSceneTex;// Main scene framebuffer image
layout(set = 0, binding = 2) uniform sampler2D depthBufferTex;// Depth buffer image

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
    vec4 color;
    if (fragUVs.x < 0.5)
    color = texture(mainSceneTex, fragUVs);
    else {
        if (fragUVs.y < 0.5){
            float depth = texture(depthBufferTex, fragUVs).r;
            color = vec4(depth, depth, depth, 1.0);
        } else {
            float depth = LinearizeDepth(texture(depthBufferTex, fragUVs).r);
            color = vec4(depth, depth, depth, 1.0);
        }
    }
    outColor = color;
}
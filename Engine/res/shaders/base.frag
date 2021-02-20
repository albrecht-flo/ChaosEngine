#version 450
#extension GL_ARB_separate_shader_objects : enable

// World uniforms, camera and world light
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 worldLightPosition;
    vec4 worldLightColor;
} ubo;

// Input from vertex shader
layout(location = 0) in vec3 fragColor; // Interpolated per vertex color
layout(location = 1) in vec3 fragNormal; // Interpolated normal vector
layout(location = 2) in vec2 fragUVs; // Interpolated texture coordinate
layout(location = 3) in vec3 fragWorldPos; // Interpolated world position of fragment
// Output to framebuffer
layout(location = 0) out vec4 outColor;

// Lighting parameters, need to be alligned to 4 floats -> vec4 with 'w' component as special parameter
struct LightSource {
    vec4  lightPos; // w = lightRadius
    vec4  lightColor; // w = ambient amount
};

layout(set = 1, binding = 0) uniform Lights {
    vec4 lightCount; // x = count; yzw unused because of alignment
    LightSource[3] sources;
} lights;

// Material Parameters
layout(set = 2, binding = 0) uniform sampler2D texSampler; // Model texture
layout(set = 2, binding = 1) uniform MaterialData { // Material properties
    float shininess;
} material;

vec3 calcLighting(vec3 lightPos, vec3 lightColor, float lightRadius, float lightAmbient, vec3 color) {
    vec3 incident = normalize(lightPos - fragWorldPos);
    // Calculate viewer vector
    vec3 viewDir =  normalize(-ubo.view[3].xyz - fragWorldPos);
    // Calculate vector between normal and perfect reflection
    vec3 reflectDir = reflect(-incident, fragNormal);
    // Calculate distance of light source to surface and attuation
    float distance = length(lightPos - fragWorldPos);
    float attenuation = 1.0 - clamp(distance / lightRadius, 0.0, 1.0);

    // Calculate diffues, and specular components
    float lambert       = max(0.0, dot(incident, fragNormal));
    float reflectionF   = max(0.0, dot(viewDir, reflectDir));
    float specularF     = pow(reflectionF, material.shininess);
    // Calculate the color components
    vec3 ambientColor = lightAmbient * lightColor;
    vec3 diffuseColor = lambert * lightColor * attenuation;
    vec3 specularColor= specularF * lightColor * attenuation;
    // Combine the color components with the fragments color
    return (ambientColor + diffuseColor + specularColor) * color;
}

// This shader calculates lighting on the model texture and
//   tints the result with the fragment color
void main() {
    // Get the base color of the fragment
    vec4 color = texture(texSampler, fragUVs);

    // Calculate world light
    vec3 rgbColor = calcLighting(
        ubo.worldLightPosition.xyz + fragWorldPos,
        ubo.worldLightColor.xyz,
        ubo.worldLightPosition.w,
        ubo.worldLightColor.w,
        color.rgb);
    // Calculate dynamic lights
    for(int i = 0; i < int(lights.lightCount.x); i++){
        rgbColor += calcLighting(
            lights.sources[i].lightPos.xyz,
            lights.sources[i].lightColor.xyz,
            lights.sources[i].lightPos.w,
            lights.sources[i].lightColor.w,
            color.rgb);
    }
    // Combine calculated texture color with fragment color // used for tinting models
    outColor = vec4(rgbColor * fragColor, color.a);
}
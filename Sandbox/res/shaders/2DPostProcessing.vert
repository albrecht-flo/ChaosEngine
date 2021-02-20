#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUVs;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUVs;

void main() {
    // Because of vertex ordering we need to invert the y component
    gl_Position = vec4(inPosition.x, -inPosition.y, 0.0, 1.0);
    fragUVs = inUVs;
}

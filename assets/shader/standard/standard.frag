#version 450 core

layout (location = 0) out vec4 color;

layout (location = 0) in vec2 oUV;
layout (location = 1) in vec3 oTint;

layout (binding = 1) uniform sampler2D texSampler;

void main() {
    // color = vec4(oUV, 0, 1);
    color = texture(texSampler, oUV) * vec4(oTint, 1);
}
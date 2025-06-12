#version 450 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec2 oUV;
layout (location = 1) out vec3 oTint;

layout (binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 tint;
} ubo;

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(vertex, 1.0);
    oUV = uv;
    oTint = ubo.tint;
}
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 color;
layout (location = 0) out vec4 OutColor;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    vec4 clr;
} ubo;


void main() {
    
    OutColor = ubo.clr;
}
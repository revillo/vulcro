#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 color;
layout (location = 0) out vec4 OutColor;

void main() {
    
    OutColor = vec4(1.0, 0.0, 0.0, 1.0);
}
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec4 OutColor;
layout (location = 0) in vec2 uv;

void main() {
  
  OutColor = vec4(uv, 0.0, 1.0);
  
}
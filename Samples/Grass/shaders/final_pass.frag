#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 OutColor;

layout (set = 0, binding = 0) uniform sampler2D colorSampler;
layout (set = 0, binding = 1) uniform sampler2D emissiveSampler;


void main() {
    
    OutColor = texture(emissiveSampler, uv).rgba;
    //OutColor = vec4(uv, 0.0, 1.0);
}
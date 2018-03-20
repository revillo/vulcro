#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//layout (location = 0) in vec4 color;
layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec4 OutEmissive;

layout (location = 0) in float height;
layout (location = 1) in vec2 uv;

const float pi = 3.141592653589793;

void main() {
        
    OutColor = mix(
      vec4(0.1, 0.3, 0.05, 1.0), 
      vec4(0.4, 0.6, 0.2, 1.0), 
    height);
    
    if (height < 0.8)
      OutColor.rgb *= 0.95 + (0.05 * pow(sin(uv.x * pi * 2.0), 2.0));
        
    OutEmissive = vec4(1.0, 1.0, 1.0, 1.0);
}
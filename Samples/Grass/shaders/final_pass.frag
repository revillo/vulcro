#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 OutColor;

layout (set = 0, binding = 0) uniform sampler2D colorSampler;
layout (set = 0, binding = 1) uniform sampler2D emissiveSampler;

float kernelSize = 6;

void main() {
    
    OutColor = texture(colorSampler, uv);
    
    if (OutColor.a < 1.0) {
      
      if (uv.y > 0.5) 
        OutColor.rgba = vec4(0.1, 0.3, 0.05, 1.0);
      else
      OutColor.rgba = mix(
        vec4(0.3, 0.6, 0.9, 1.0) * 0.6,
        vec4(0.7, 0.9, 1.0, 1.0), uv.y * 1.4);
    }
    
    vec4 blur = vec4(0.0, 0.0, 0.0, 0.0);
    
    float weight = 0.0;
    
    for (float dx = -kernelSize; dx <= kernelSize; dx += 1.0f)
    for (float dy = -kernelSize; dy <= kernelSize; dy += 1.0f)
    {
      vec2 sampleUV = uv + vec2(dx, dy) * vec2(1.0/1000.0, 1.0/500.0);
      
      if (sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0) continue;
        
      float amt = 1.0 / (pow(length(vec2(dx, dy)), 2.0) + 1.0);
      weight += amt;
      
      blur += texture(emissiveSampler, sampleUV) * amt;
    }
    
    OutColor += blur * 0.1;
    OutColor.a = 1.0;

}
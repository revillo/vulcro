#version 430
#extension GL_GOOGLE_include_directive : require

//uniform layout (set = 0, binding = 0, rgba8) image2D RayImage;
uniform layout (set = 0, binding = 0) sampler2D RayImage;
layout (location = 0) in vec2 uv;

layout(location = 0) out vec4 OutColor;

void main() {
  
  //OutColor = vec4(uv, 1.0, 1.0);
  
  //ivec2 pixel = ivec2(uv * vec2(511, 511));
  ivec2 pixel = ivec2(gl_FragCoord.xy);
  
  //imageStore(RayImage, pixel, vec4(uv, 1.0, 1.0)); 

  //imageAtomicExchange(RayImage, pixel, 1000); 
  
  OutColor = texture(RayImage, uv);
  //OutColor = vec4(pixel / 512.0, 1.0, 1.0);
  //OutColor.b = 0.5;
  //OutColor.a = 1.0;
}
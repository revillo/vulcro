#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//layout (location = 0) in vec4 color;
layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec4 OutEmissive;

layout (location = 0) in float height;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 eyePos;
layout (location = 3) in vec4 worldPos;

layout (set = 0, binding = 0) uniform SceneGlobals{
	mat4 perspective;
	mat4 view;
	vec4 viewPos;
	vec4 sunDirWorld;
	vec4 time;
} uScene;

const float pi = 3.141592653589793;

void main() {
        
    
    vec3 X = dFdx(worldPos.xyz);
    vec3 Y = dFdy(worldPos.xyz);
    vec3 normal = normalize(cross(X,Y));    
    
   
    
    if (height < 0.8)
      OutColor.rgb *= 0.9 + (0.1 * pow(sin(uv.x * pi * 4.0), 2.0));
        
    //dot(normal, uScene.sunDirWorld.xyz);    
      
    vec3 eyeRay = normalize(uScene.viewPos.xyz - worldPos.xyz);
    
    vec3 refl = reflect(eyeRay, normal);
    
    float diffuse = dot(normal, uScene.sunDirWorld.xyz) * 0.2 + 1.0;
    float spec = pow(max(0.0, dot(refl, uScene.sunDirWorld.xyz)), 10.0);
      
    OutEmissive = vec4(vec3(0.5, 0.65, 0.2) * spec * 1.5, spec);
    
    vec4 upper = vec4(0.5, 0.65, 0.2, 1.0) * diffuse * 1.3;
    
    OutColor = mix(
      vec4(0.1, 0.3, 0.05, 1.0), 
      upper, 
    height);
    OutColor.a = 1.0;
}
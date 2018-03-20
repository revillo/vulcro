#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 pos;

//layout (location = 0) out vec4 outColor;

layout (location = 0) out float nHeight;
layout (location = 1) out vec2 uv;

layout (set = 0, binding = 0) uniform SceneGlobals{
  mat4 perspective;
  mat4 view;
} uScene;

const int gridSize = 200;
const float gridSizeF = float(gridSize);

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

float random(vec2 p){return fract(cos(dot(p,vec2(23.14069263277926,2.665144142690225)))*12345.6789);}

void main() {
   nHeight = pos.y;

   uv.x = pos.x;
   uv.y = pos.y;
   
   float taper = 1.0 - nHeight;
   taper = pow(taper, 0.5);
   
   int x = gl_InstanceIndex / gridSize;
   int z = gl_InstanceIndex % gridSize;
   float rand = random(vec2(x / gridSizeF, z / gridSizeF));
   
   vec4 tilt = pos;
   
   tilt.z += nHeight * nHeight * 0.6;
   tilt.x *= taper;
   
   vec4 newPos = rotationMatrix(vec3(0.0, 1.0, 0.0), rand * 10.0) * tilt;

   
   newPos.x += x * 1.0;
   newPos.z += z * 1.0;
   newPos.y *= 6.0 + 2.0 * rand;
   
   newPos.xyz -= vec3(gridSizeF, 0.0, gridSizeF) * 0.5;
   
   
   
   gl_Position = uScene.perspective * uScene.view * newPos;
}
#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "common.h"
#include "noise.glsl"

layout (set = 0, binding = 1) uniform accelerationStructureNV RayScene;


layout(location = RAY_LOCATION) rayPayloadInNV RayPayload PrimaryRay;
                                hitAttributeNV vec2 hitData;

layout(location = SHADOW_RAY_LOCATION) rayPayloadInNV ShadowRayPayload ShadowRay;
                                
layout (set = 0, binding = 3) buffer VertexList {
  
  Vertex v[];

} VertexData;

layout (set = 0, binding = 4) buffer IndexList {
  
  uint i[];

} IndexData;

const vec3 hemivecs[16] = vec3[](
  vec3(0.13820,0.42533,0.89443),
  vec3(-0.36180,0.26287,0.89443),
  vec3(-0.36180,-0.26287,0.89443),
  vec3(0.13820,-0.42533,0.89443),
  vec3(0.19544,0.60150,0.77460),
  vec3(-0.51167,0.37175,0.77460),
  vec3(-0.51167,-0.37175,0.77460),
  vec3(0.19544,-0.60150,0.77460),
  vec3(0.23936,0.73669,0.63246),
  vec3(-0.62666,0.45530,0.63246),
  vec3(-0.62666,-0.45530,0.63246),
  vec3(0.23936,-0.73669,0.63246),
  vec3(0.27639,0.85065,0.44721),
  vec3(-0.72361,0.52573,0.44721),
  vec3(-0.72361,-0.52573,0.44721),
  vec3(0.27639,-0.85065,0.44721)
);

#define INSTANCE nonuniformEXT(gl_InstanceCustomIndexNV)
#define FACE gl_PrimitiveID
#define VERTICES VertexData.v       
#define INDICES IndexData.i       
#define EYE_RAY gl_ObjectRayDirectionNV 

const float SOLAR_ENERGY =  2.0;
const float AMBIENT_ENERGY = 1.0;

layout (set = 0, binding = 0) uniform GlobalData{
  
  Globals g;
  
} uGlobals;

#define GLOBALS uGlobals.g
                      
void main() {
  vec3 barys = vec3(1 - hitData.x - hitData.y, hitData.x, hitData.y) ;
  
  Vertex v1 = VERTICES[INDICES[FACE * 3]];
  Vertex v2 = VERTICES[INDICES[FACE * 3 + 1]];
  Vertex v3 = VERTICES[INDICES[FACE * 3 + 2]];
  
  vec3 normal = normalize(cross(normalize(v2.position.xyz - v1.position.xyz), normalize(v3.position.xyz - v1.position.xyz)));
  Vertex vlerp = LerpFace(v1, v2, v3, barys);

  
  
   const uint rayFlags = gl_RayFlagsCullFrontFacingTrianglesNV;
   const uint cullMask = 0xFF;
   
   vec3 origin = vlerp.position.xyz;
   vec3 direction = GLOBALS.sunDir.xyz;
   
   //Water normal
   if (vlerp.material.x > 0.01) {
     normal = mix(normal, normalNoise(vec2(origin.x / 3.0, origin.z / 3.0), GLOBALS.time * 0.1), 0.04); 
     normal = normalize(normal);
  }
   
   float sunDiffuse = max(0.0, dot(direction, normal));// * 0.7 + 0.3;
   
   float tmin = 0.001;
   float tmax = 100.0;
  ShadowRay.occlusion = 0.0;
   
   //Sun Lighting
   traceNV(
       RayScene,
       rayFlags,
       cullMask,
       1,//record offset
       1,//record stride
       1,//miss index
       origin,
       tmin,
       direction,
       tmax,
        SHADOW_RAY_LOCATION
    );
    
    
 float sunEnergy = (1.0 - ShadowRay.occlusion) * sunDiffuse * SOLAR_ENERGY;
 
 //Ambient lighting
 
 float ambientEnergy = 1.0;
 
  tmax = 0.5;
  
 vec3 randvec = normalize(vec3(normal.y, -normal.z, normal.x));
 vec3 tangent = normalize(randvec - dot(randvec, normal) * normal);
 mat3 aligned_mat = mat3(tangent, normalize(cross(normal, tangent)), normal);


  for (uint i = 0; i < 16; i++) {
     
      traceNV(
       RayScene,
       rayFlags,
       cullMask,
       1,//record offset
       1,//record stride
       1,//miss index
       origin,
       tmin,
       aligned_mat * hemivecs[i],
       tmax,
        SHADOW_RAY_LOCATION
      );
      
      ambientEnergy -= ShadowRay.occlusion / 16.0;
  }
  
  ambientEnergy *= AMBIENT_ENERGY;
  
  vec3 color = vlerp.color.rgb;
  //vec3 pixels = random3(floor(origin * 4.0) / 4.0);
  vec3 pixels = random3(ivec3(floor(origin * 8.0)) / 8.0);  
  color += pixels * 0.2;
 
  float energy = (sunEnergy + ambientEnergy) / (SOLAR_ENERGY + AMBIENT_ENERGY);
  //Reflection
   
 
  if (vlerp.material.x > 0.01) {

    direction = reflect(EYE_RAY, normal);
    tmax = 100.0;
  
    traceNV(
       RayScene,
       rayFlags,
       cullMask,
       0,//record offset
       1,//record stride
       0,//miss index
       origin,
       tmin,
       direction,
       tmax,
       RAY_LOCATION
      );
      
      float sunSpec = pow(max(0.0, dot(direction, GLOBALS.sunDir)), 30.0);
      
      color = mix(color, PrimaryRay.color, vlerp.material.x);
      color += vec3(sunSpec);
  }
  
  
  //Refract transparency
  direction = refract(EYE_RAY, normal, 0.8);

  if (vlerp.color.a < 0.9 && length(direction) > 0.01) {
    
    tmax = 10.0;
  
    traceNV(
       RayScene,
       rayFlags,
       cullMask,
       0,//record offset
       1,//record stride
       0,//miss index
       origin,
       tmin,
       direction,
       tmax,
       RAY_LOCATION
      );
      
      float fog = clamp(PrimaryRay.hitDistance, 0.0, tmax) / tmax;
     // fog = 0.0;
      
      color = mix(PrimaryRay.color, color, vlerp.color.a);
      color = mix(color, vec3(0.3, 0.3, 0.1), fog);
      //color = PrimaryRay.color;
    
  }
  
  PrimaryRay.color = color * energy;
  PrimaryRay.hitDistance = gl_HitTNV;

  
}
#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "common.h"
layout(location = RAY_LOCATION) rayPayloadInNV RayPayload PrimaryRay;
                                hitAttributeNV vec2 hitData;

                                
layout (set = 0, binding = 2) buffer VertexList {
  
  Vertex v[];

} VertexData;

#define INSTANCE nonuniformEXT(gl_InstanceCustomIndexNV)
#define VERTICES VertexData.v       
                      
void main() {
  vec3 barys = vec3(1 - hitData.x - hitData.y, hitData.x, hitData.y) ;
  
  Vertex v1 = VERTICES[INSTANCE * 3];
  Vertex v2 = VERTICES[INSTANCE * 3 + 1];
  Vertex v3 = VERTICES[INSTANCE * 3 + 2];
  
  Vertex vlerp = LerpFace(v1, v2, v3, barys);
  
  PrimaryRay.color = vlerp.color.rgb;
  PrimaryRay.hitDistance = gl_HitTNV;
}
#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "common.h"
layout(location = RAY_LOCATION) rayPayloadInNV ShadowRayPayload ShadowRay;

void main() {
  
  ShadowRay.occlusion = 0.0;
  //ShadowRay.hitDistance = 10000.0;

}
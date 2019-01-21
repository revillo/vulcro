#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "common.h"
layout(location = RAY_LOCATION) rayPayloadInNV RayPayload PrimaryRay;
                                hitAttributeNV vec2 hitData;

void main() {
  vec3 barys = vec3(1 - hitData.x - hitData.y, hitData.x, hitData.y) ;

  PrimaryRay.color = vec4(barys, 1.0);
}
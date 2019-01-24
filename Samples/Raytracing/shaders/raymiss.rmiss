#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "common.h"
layout(location = RAY_LOCATION) rayPayloadInNV RayPayload PrimaryRay;

void main() {
  PrimaryRay.hitDistance = -1.0;
}
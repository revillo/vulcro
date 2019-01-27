#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "common.h"
layout(location = RAY_LOCATION) rayPayloadInNV RayPayload PrimaryRay;

layout (set = 0, binding = 0) uniform GlobalData{
  
  Globals g;
  
} uGlobals;

#define GLOBALS uGlobals.g
#define RAY_DIR gl_ObjectRayDirectionNV 

const vec3 skyBlueTop = vec3(0.0, 0.8, 1.0);
const vec3 skyBlueBottom = vec3(0.0, 0.45, 0.91);
const vec3 skySunsetTop = vec3(0.6, 0.5, 0.4);
const vec3 skySunsetBottom = vec3(0.9, 0.8, 0.0);
const vec3 skyNightTop = vec3(0.1, 0.1, 0.2);
const vec3 skyNightBottom = vec3(0.14, 0.14, 0.24);
const vec3 sunColor = vec3(1.0, 1.0, 0.9);
void main() {

    float playerAngle = 1.0;
    float sunAngle = dot(RAY_DIR, GLOBALS.sunDir.xyz) * 0.5 + 0.5;

    vec3 skyColor = mix(
      mix(skySunsetTop, skyBlueTop, playerAngle) ,
      mix(skySunsetBottom, skyBlueBottom, playerAngle) ,
    sunAngle);
    
    skyColor += sunColor * pow(max(0.0, sunAngle), 40.0);

  PrimaryRay.color = skyColor;
  PrimaryRay.hitDistance = -1.0;
}
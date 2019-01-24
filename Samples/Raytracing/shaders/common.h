#define RAY_LOCATION 1

struct RayPayload {
  vec3 color;
  float hitDistance;
};

struct Vertex {
  vec4 position;
  vec4 color;
};

vec2 BaryLerp(vec2 a, vec2 b, vec2 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec3 BaryLerp(vec3 a, vec3 b, vec3 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec4 BaryLerp(vec4 a, vec4 b, vec4 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

Vertex LerpFace(Vertex a, Vertex b, Vertex c, vec3 barycentrics) {
    Vertex result;
    result.position = BaryLerp(a.position, b.position, c.position, barycentrics);
    result.color = BaryLerp(a.color, b.color, c.color, barycentrics);
    return result;
}
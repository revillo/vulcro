#version 430
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) out vec2 uv;

vec2 verts[4] = vec2[] (
  
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, 1.0)

);


void main() {

  uv = verts[gl_VertexIndex];
  
  gl_Position = vec4(uv, 0.0, 1.0);
  
  uv = uv * 0.5 + vec2(0.5);

}
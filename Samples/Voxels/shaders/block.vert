#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 apos;
layout (location = 1) in vec2 auv;

layout (location = 0) out vec2 uv;

const int GRID_SIDE = 20;
const int GRID_SIDE2 = GRID_SIDE * GRID_SIDE;
const int GRID_COUNT = GRID_SIDE * GRID_SIDE2;

const float BLOCK_SIZE = 2.0;

layout (set = 0, binding = 0) uniform SceneGlobals {
  mat4 perspective;
  mat4 view;
} uScene;


layout (set = 1, binding = 0) uniform BlockType {
  ivec4 type[250];
} grid;

int getType(int index) {
  return grid.type[index / 4][index % 4];
}

void main() {
  
  
  if (getType(gl_InstanceIndex) == 0) {
    
    gl_Position = vec4(0.0, 0.0, -1.0, 1.0);
    return;
    
  }
  
  int x = gl_InstanceIndex / GRID_SIDE2;
  int y = gl_InstanceIndex % GRID_SIDE2 / GRID_SIDE;
  int z = gl_InstanceIndex % GRID_SIDE2 % GRID_SIDE;

  vec4 worldPos = vec4(apos, 1.0);
  uv = auv;
  
  worldPos.x += float(x) * BLOCK_SIZE;
  worldPos.y += float(y) * BLOCK_SIZE;
  worldPos.z += float(z) * BLOCK_SIZE;
  
  gl_Position = uScene.perspective * uScene.view * worldPos;
  
}


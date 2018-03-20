#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 pos;

//layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform SceneGlobals{
  mat4 perspective;
  mat4 view;
} uScene;

void main() {

   gl_Position = uScene.perspective * uScene.view * pos;
   
}
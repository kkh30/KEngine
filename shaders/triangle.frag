#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec4 inViewPos;
layout (location = 2) in vec4 inViewLightPos;

layout (location = 0) out vec4 outFragColor;


void main() 
{


  vec3 ambient = vec3(0.15);
  vec3 eye_dir = -inViewPos.xyz;
  vec3 light_dir = -inViewLightPos.xyz + inViewPos.xyz;
  vec3 h = eye_dir+light_dir;
  h /= length(h);
  float diffuse = max(0,dot(normalize(inNormal),normalize(light_dir)));
  float specular = max(0,dot(inNormal,h));
  outFragColor = vec4((ambient + (diffuse + specular * 0.0002)* vec3(0.75)),1.0);
}
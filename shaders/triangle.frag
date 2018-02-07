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
  vec3 normal = normalize(inNormal);
  vec3 eye_dir = -inViewPos.xyz;
  vec3 light_dir = inViewLightPos.xyz - inViewPos.xyz;
  float light_distance = length(light_dir);
  //light_dir = normalize(light_dir);
  vec3 h = normalize(eye_dir+light_dir);
  float diffuse = clamp(dot(normal,normalize(light_dir)),0.0,1.0);
  float specular = pow(clamp(dot(normal,normalize(h)),0.0,1.0),25.0);
  outFragColor = vec4(ambient + (diffuse * vec3(0.75)),1.0);
}
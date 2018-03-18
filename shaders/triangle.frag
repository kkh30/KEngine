#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec4 inViewPos;
layout (location = 2) in vec4 inViewLightPos;
layout (location = 3) in vec4 shadow_coord;
layout (location = 4) in vec4 inMaterial;

layout (location = 0) out vec4 outFragColor;
layout (set = 0,binding = 1) uniform sampler2DShadow shadowmap;



void main() 
{


  vec4 ambient = vec4(0.15,0.15,0.15,1.0);
  vec3 normal = normalize(inNormal);
  vec3 eye_dir = -inViewPos.xyz;
  vec3 light_dir = inViewLightPos.xyz - inViewPos.xyz;
  float light_distance = length(light_dir);
  //light_dir = normalize(light_dir);
  vec3 h = normalize(eye_dir+light_dir);
  float diffuse = clamp(dot(normal,normalize(light_dir)),0.0,1.0);
  float specular = pow(clamp(dot(normal,normalize(h)),0.0,1.0),20.0);
  float shadow = textureProj(shadowmap, shadow_coord);
  outFragColor = ambient + inMaterial * shadow * (diffuse * 0.45 + specular) ;
}
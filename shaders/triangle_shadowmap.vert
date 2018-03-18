#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (set = 0,binding = 0) uniform MVP{
	mat4 proj;
	mat4 view;
	mat4 model;
	mat4 shadow_view;

} mvp;

layout (set = 1,binding = 0) uniform PerComponentUniform{
	mat4 model;
	vec4 material;	
} per_component_uniform;

vec3 lightPos = vec3(100.0,100.0,100);

out gl_PerVertex 
{
    vec4 gl_Position;   
};



void main() 
{
	mat4 viewModelMatrix = mvp.shadow_view * mvp.model * per_component_uniform.model;
	gl_Position = mvp.proj * viewModelMatrix * vec4(inPos.xyz, 1.0);
}

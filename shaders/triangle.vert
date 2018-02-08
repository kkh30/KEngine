#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (set = 0,binding = 0) uniform MVP{
	mat4 proj;
	mat4 view;
	mat4 model;
} mvp;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec4 outViewPos;
layout (location = 2) out vec4 outViewLightPos;

vec3 lightPos = vec3(100.0,100.0,100);

out gl_PerVertex 
{
    vec4 gl_Position;   
};


void main() 
{
	mat4 viewModelMatrix = mvp.view * mvp.model;
	outNormal = (transpose(inverse(viewModelMatrix)) * vec4(inNormal,0)).xyz;
	//outNormal = (mvp.view * mvp.model * vec4(inNormal,0)).xyz;
	
	outViewPos = viewModelMatrix *vec4(inPos,1);
	outViewLightPos = mvp.view *vec4(lightPos,1);
	gl_Position = mvp.proj * viewModelMatrix * vec4(inPos.xyz, 1.0);
}

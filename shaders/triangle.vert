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

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec4 outViewPos;
layout (location = 2) out vec4 outViewLightPos;
layout (location = 3) out vec4 shadow_coord;
layout (location = 4) out vec4 outMaterial;

vec3 lightPos = vec3(10.0,10.0,10.0);
const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );


out gl_PerVertex 
{
    vec4 gl_Position;   
};


void main() 
{
	outMaterial = per_component_uniform.material;
	mat4 viewModelMatrix = mvp.view * mvp.model * per_component_uniform.model ;
	outNormal = (transpose(inverse(viewModelMatrix)) * vec4(inNormal,0)).xyz;
	//outNormal = (mvp.view * mvp.model * vec4(inNormal,0)).xyz;
	shadow_coord = biasMat * mvp.proj * mvp.shadow_view * mvp.model * per_component_uniform.model * vec4(inPos.xyz, 1.0);
	outViewPos = viewModelMatrix *vec4(inPos,1);
	outViewLightPos = mvp.view *vec4(lightPos,1);
	gl_Position = mvp.proj * viewModelMatrix * vec4(inPos.xyz, 1.0);
}

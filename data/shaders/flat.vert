#version 450
#extension GL_ARB_separate_shader_objects : enable
//#extension GL_EXT_debug_printf : enable

layout(set = 0, binding = 0) uniform UniformBufferObject{
	mat4  view;
	mat4  proj;
	vec4  lights[10];
	vec4  viewPos;
	vec2  screen;
	vec2  mousepos;
	vec3  mouseWorld;
	float time;
} ubo;

layout(push_constant) uniform PushConsts{
	mat4 model;
} primitive;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec3 outNormal;

void main() {
    vec3 light = vec3(ubo.viewPos);
	
	//debugPrintfEXT("%f", ubo.time);
	
	vec3 normal = mat3(primitive.model) * inNormal;
	vec3 position = primitive.model[3].xyz;
	
    gl_Position = ubo.proj * ubo.view * primitive.model * vec4(inPosition.xyz, 1.0);
	outColor = vec3(clamp(dot(normalize(light - position), normal) * 0.7, .1f, 1),
					clamp(dot(normalize(light - position), normal) * 0.7, .1f, 1),
					clamp(dot(normalize(light - position), normal) * 0.7, .1f, 1));
	outTexCoord = inTexCoord;
	outNormal = normal;
}
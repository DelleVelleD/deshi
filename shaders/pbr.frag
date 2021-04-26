#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D albedoSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D specularSampler;
layout(set = 1, binding = 3) uniform sampler2D lightSampler;

//layout(constant_id = 0) const bool ALPHA_MASK = false;
//layout(constant_id = 1) const float ALPHA_THRESHOLD = 0.0f;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inPosition;
layout(location = 4) in float time;
layout(location = 5) in vec3 fragPos;
layout(location = 6) in float width;
layout(location = 7) in float height;


layout(location = 0) out vec4 outColor;

float interp(float a, float b, float t){
	return (1 - t) * a + t * b;
}

void main() {
	//TODO(delle,r) pbr shading, see pbrtexture in saschawillems' stuff
	vec2 tc = inTexCoord;
	vec4 fc = gl_FragCoord;

	fc.x /= width;
	fc.y /= height;

	tc.y *= -1;
	vec4 normal = texture(albedoSampler, tc);
	vec4 tex = texture(albedoSampler, tc);
	vec3 light = vec3(-8000, 2 ,2);
	vec4 lightcolor = vec4(
					  clamp(dot(normalize(light - inPosition), -vec3(normal)), 0, 1),
					  clamp(dot(normalize(light - inPosition), -vec3(normal)), 0, 1),
					  clamp(dot(normalize(light - inPosition), -vec3(normal)), 0, 1), 1);
	
	
	outColor = tex;// * lightcolor;// * color;// * color2;

}
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main() {
    //outColor = vec4(inTexCoord, 1.0);
    //TODO(r,delle) combine normal with light
//https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/glsl/pipelines/phong.frag
	outColor = texture(texSampler, inTexCoord);
}
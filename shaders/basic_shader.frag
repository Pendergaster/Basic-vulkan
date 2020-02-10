/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 inUv;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, inUv) + vec4(fragColor, 1.0) * 0.5;
    //outColor = vec4(inUv, 0.0, 1.0);
    //outColor = vec4(fragColor, 1.0);
}


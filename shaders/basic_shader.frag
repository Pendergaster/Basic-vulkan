/************************************************************
* Check license.txt in project root for license information *
*********************************************************** */

#version 450
#extension GL_ARB_separate_shader_objects : enable
// Out
layout(location = 0) out vec4 outColor;

// In
layout(location = 0) in vec3 color;

void main() {
    outColor = vec4(color, 1.0);
}

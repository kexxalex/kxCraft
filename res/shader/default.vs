#version 450 core

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aUV;
layout(location=2) in vec3 aNormal;

uniform mat4x4 MVP;

out vec2 vUV;
out vec3 vNormal;

void main() {
    vNormal = aNormal;
    vUV = aUV;

    gl_Position = MVP * vec4(aPosition, 1.0);
}
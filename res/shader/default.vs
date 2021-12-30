#version 330 core

layout(location=0) in vec3 aPosition;

uniform mat4x4 MVP;

void main() {
    gl_Position = MVP * vec4(aPosition, 1.0);
}
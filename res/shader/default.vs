#version 330 core

layout(location=0) in vec3 aPosition;
layout(location=1) in float aLight;

uniform vec3 CHUNK_POSITION;
uniform mat4 MVP;

out float vTexture;

void main() {
    vTexture = aLight;
    gl_Position = MVP * vec4(CHUNK_POSITION + aPosition.xyz, 1.0);
}
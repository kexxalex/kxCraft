#version 330 core

layout(location=0) out vec3 outFragColor;
in float vTexture;

void main() {
    outFragColor = vec3(vTexture * 0.016666666666);
} 
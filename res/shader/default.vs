#version 330 core

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 textureWithOcclusion;

out int vTexture;
out int vLight;

void main() {
    vTexture = int(textureWithOcclusion.x);
    vLight = int(textureWithOcclusion.y);
    gl_Position = vec4(aPosition, 1.0);
}
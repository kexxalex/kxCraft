#version 450 core

layout(location=0) in uvec4 aPositionMode;
layout(location=1) in uvec4 aLight;
layout(location=2) in uint aID;
layout(location=3) in vec3 aChunkPosition;

out uint vMode;
out uint vTexture;
out uvec4 vLight;

void main() {
    vMode = aPositionMode.a;
    vTexture = aID;
    vLight = aLight;

    gl_Position = vec4(aChunkPosition + aPositionMode.xyz, 1.0);
}
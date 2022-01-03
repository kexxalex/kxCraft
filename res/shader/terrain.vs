#version 450 core

layout(location=0) in vec4 aPositionWithID;
layout(location=1) in float aLight;
layout(location=2) in vec3 aChunkPosition;

uniform vec3 PLAYER_POSITION;

out int vTexture;
out int vLight;

void main() {
    vTexture = int(aPositionWithID.w);
    vLight = int(aLight);

    gl_Position = vec4(aChunkPosition * 16.0 + aPositionWithID.xyz, 1.0);
}
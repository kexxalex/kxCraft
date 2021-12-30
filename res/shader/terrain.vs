#version 330 core

layout(location=0) in vec4 aPositionWithID;
layout(location=1) in float aLight;

uniform vec3 CHUNK_POSITION;

out int vTexture;
out int vLight;

void main() {
    vTexture = int(aPositionWithID.w);
    vLight = int(aLight);
    gl_Position = vec4(CHUNK_POSITION + aPositionWithID.xyz, 1.0);
}
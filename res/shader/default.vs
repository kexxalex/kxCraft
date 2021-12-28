#version 330 core

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 textureWithOcclusion;

uniform mat4 MVP;

out int gTexture;
out int gLight;

void main()
{
    gTexture = int(textureWithOcclusion.x);
    gLight = int(textureWithOcclusion.y);
    gl_Position = vec4(aPosition, 1.0);
}
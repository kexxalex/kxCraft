#version 450 core

layout(binding=0) uniform sampler2D DIFFUSE;
layout(binding=1) uniform sampler2D ITEMS;

layout(location=0) out vec3 outFragColor;
in vec2 vUV;
in vec3 vNormal;

void main() {
    vec4 color = texture(DIFFUSE, vUV);
    if (color.a < 0.5)
        discard;
    outFragColor = color.rgb;
} 
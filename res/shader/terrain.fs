#version 420 core
layout(location=0) out vec3 outFragColor;

layout(binding=0) uniform sampler2D DIFFUSE;

uniform float INV_RENDER_DIST;
uniform float TIME;
uniform vec3 PLAYER_POSITION;

in vec3 gNormal;
in vec2 gFace;
in vec2 gUV;
in vec3 gFragPosition;
in vec4 gLight;

void main() {
    vec4 diffColor = texture(DIFFUSE, gUV);
    if (diffColor.a < 0.5f)
        discard;


    float bilinearLight = (gFace.x * gFace.y * gLight.a +
        (1.0f - gFace.x) * (1.0f - gFace.y) * gLight.r +
        gFace.x * (1.0f - gFace.y) * gLight.g +
        (1.0f - gFace.x) * gFace.y * gLight.b
    );

    // Directional sun light
    vec3 lightDir = normalize(vec3(0.5f, -1.0f, 0.25f));
    float dLight = min(1.2f, max(0.0f, dot(-lightDir, gNormal)) * 0.7f + 0.4f);

    // Ambient Light from occlusion
    float aLight = min(bilinearLight, 1.0f) * 0.95f + 0.05;


    vec3 delta = (PLAYER_POSITION - gFragPosition) * INV_RENDER_DIST;
    float fog = 1.0f - clamp(dot(delta, delta)*4.0 - 3.0, 0.0, 1.0f);
    outFragColor = mix(vec3(0.75, 0.9, 1.0), aLight*dLight * diffColor.rgb, fog);
} 
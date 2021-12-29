#version 330 core
layout(location = 0) out vec3 outFragColor;

uniform sampler2D DIFFUSE;

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

    vec3 lightDir = normalize(vec3(0.5f, -1.0f, 0.25f));

    float bilinearLight = (gFace.x * gFace.y * gLight.a +
        (1.0f - gFace.x) * (1.0f - gFace.y) * gLight.r +
        gFace.x * (1.0f - gFace.y) * gLight.g +
        (1.0f - gFace.x) * gFace.y * gLight.b
    ) * 0.015625f;

    // Directional sun light
    vec3 lightDirection = normalize(vec3(0.75, -1.0, 0.25)); //vec3(sin(TIME * 0.125f), cos(TIME * 0.125f), 0.0f);
    float dLight = min(1.2f, max(0.0f, dot(-lightDirection, gNormal)) * 0.9f + 0.3f);

    // Ambient Light from occlusion
    float aLight = smoothstep(0.0, 1.0, min(bilinearLight, 1.0f) * 0.8f + 0.2f);


    vec3 delta = (PLAYER_POSITION - gFragPosition) * INV_RENDER_DIST;
    float fog = 1.0f - min(dot(delta, delta), 1.0f);
    outFragColor = mix(vec3(0.8, 0.95, 1.0), aLight*dLight * diffColor.rgb, fog);
} 
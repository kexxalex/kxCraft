#version 450 core
layout(location=0) out vec3 outFragColor;

layout(binding=0) uniform sampler2D DIFFUSE;
layout(binding=1) uniform sampler2D NORMAL;
layout(binding=2) uniform sampler2D SPECULAR;

uniform float INV_RENDER_DIST;
uniform float TIME;
uniform vec3 PLAYER_POSITION;
uniform bool HUD;

in vec3 gNormal;
in vec2 gFace;
in vec2 gUV;
in vec3 gFragPosition;
in vec4 gLight;
flat in int gTextureID;

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
    vec3 lightDir = normalize(vec3(0.25f, -1.0f, 0.25f));
    float dLight = 1.0;

    // Ambient Light from occlusion
    float aLight = 1.0;
    if (HUD) {
        dLight = min(1.0f, max(0.0f, dot(-lightDir, gNormal)) * 0.6f + 0.5f);
    }
    else {
        dLight = max(dot(-lightDir, gNormal), 0.0) * 0.2f + 0.8f;
        aLight = bilinearLight + 0.05;
    }


    vec3 delta = (PLAYER_POSITION - gFragPosition) * INV_RENDER_DIST;
    float fog = 1.0f - clamp(dot(delta, delta)*2.0 - 1.0, 0.0, 1.0f);
    outFragColor = mix(vec3(0.75, 0.9, 1.0), aLight*dLight * diffColor.rgb, fog); //
} 
#version 450 core
layout(location=0) out vec3 outFragColor;

layout(binding=0) uniform sampler2D DIFFUSE;
layout(binding=1) uniform sampler2D NORMAL;
layout(binding=2) uniform sampler2D SPECULAR;

uniform float INV_RENDER_DIST;
uniform float TIME;
uniform vec3 EYE_POSITION;
uniform bool HUD;

in mat3 gTBN;

in vec2 gFace;
in vec2 gUV;
in vec3 gFragPosition;
in vec4 gLight;
flat in int gTextureID;

void main() {
    vec4 diffColor = texture(DIFFUSE, gUV);
    if (diffColor.a < 0.5f)
        discard;

    vec3 uv_normal = normalize(texture(NORMAL, gUV).rgb * 2.0 - 1.0);
    float spec = texture(SPECULAR, gUV).r;

    vec3 normal = gTBN * uv_normal;

    // Directional sun light
    vec3 lightDir = normalize(vec3(-1,-1,-1));
    float dLight = 1.0;
    float sLight = 0.0;

    // Ambient Light from occlusion
    float aLight = 1.0;
    if (HUD) {
        dLight = max(-dot(lightDir, gTBN[2].xyz), 0.0) * 0.6 + 0.5;
    }
    else {
        float bilinearLight = (gFace.x * gFace.y * gLight.a +
            (1.0f - gFace.x) * (1.0f - gFace.y) * gLight.r +
            gFace.x * (1.0f - gFace.y) * gLight.g +
            (1.0f - gFace.x) * gFace.y * gLight.b
        );

        vec3 view = normalize(gFragPosition - EYE_POSITION);

        // Directional Light
        dLight = max(-dot(lightDir, normal), 0.0) * 0.3 + 0.8;

        // Specular Highlights
        sLight = (dLight > 0.8) ? max(-dot(reflect(view, normal), lightDir) * spec, 0.0) : 0.0;
        aLight = bilinearLight + 0.05;
    }


    vec3 delta = (EYE_POSITION - gFragPosition) * INV_RENDER_DIST;
    float fog = 1.0f - clamp(dot(delta, delta)*2.0 - 1.0, 0.0, 1.0f);

    float totalLight = aLight * dLight * (1.0+sLight);

    outFragColor = mix(vec3(0.75, 0.9, 1.0), totalLight * diffColor.rgb, fog);
} 
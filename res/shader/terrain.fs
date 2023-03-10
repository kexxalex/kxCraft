#version 450 core
layout(location=0) out vec3 outFragColor;

layout(binding=0) uniform sampler2D DIFFUSE;
layout(binding=1) uniform sampler2D NORMAL;
layout(binding=2) uniform sampler2D SPECULAR;
layout(binding=3) uniform sampler2D OCCLUSION;

uniform float INV_RENDER_DIST;
uniform float TIME;
uniform vec3 EYE_POSITION;
uniform bool HUD;

in mat3 gTBN;

in vec2 gFace;
in vec2 gUV;
in vec3 gFragPosition;
in vec4 gLight;
in vec3 gLightDir;
flat in uint gTextureID;

vec3 ACESFilm(in vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec3 Reinhard(in vec3 x) {
    return x / (x + 1.0);
}

void main() {
    vec4 diffColor = texture(DIFFUSE, gUV);
    if (diffColor.a < 0.5)
        discard;

    vec3 uv_normal = normalize(texture(NORMAL, gUV).rgb * 2.0 - 1.0);
    float spec = texture(SPECULAR, gUV).r;
    //float occl = texture(OCCLUSION, gUV).r;

    vec3 normal = gTBN * uv_normal;

    // Directional sun light
    float totalLuma = 0.4;
    float dLight = 1.0;
    float sLight = 0.0;

    // Ambient Light with occlusion
    float aLight = 1.0;
    if (HUD) {
        dLight = max(-dot(gLightDir, normal), 0.0) * 1.5 + 0.5;
    }
    else {
        float bilinearLight = (gFace.x * gFace.y * gLight.a +
            (1.0f - gFace.x) * (1.0f - gFace.y) * gLight.r +
            gFace.x * (1.0f - gFace.y) * gLight.g +
            (1.0f - gFace.x) * gFace.y * gLight.b
        );

        vec3 view = normalize(gFragPosition - EYE_POSITION);

        // Directional Light
        dLight = clamp(max(-dot(gLightDir, normal), 0.0) * 0.75 + 0.25, 0.0, 1.0);
        sLight = pow(clamp(dot(reflect(view, normal), gLightDir), 0.0, 1.0), 5.0) * spec * 4.0;

        aLight = bilinearLight;
    }


    vec3 delta = (EYE_POSITION - gFragPosition) * INV_RENDER_DIST;
    float fog = 1.0f - clamp(dot(delta, delta)*2.0 - 1.0, 0.0, 1.0f);

    //float totalLight = totalLuma * mix(occl*aLight, aLight*dLight+occl*aLight*sLight, dLight);
    float totalLight = mix(0.01, totalLuma * aLight, dLight) + sLight * aLight * dLight;

    vec3 mapped = ACESFilm(totalLight * diffColor.rgb);
    //
    outFragColor = mix(vec3(0.75, 0.9, 1.0), pow(mapped, vec3(0.4545454545)), fog);
} 

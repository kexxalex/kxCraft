#version 330 core
out vec3 outFragColor;

uniform sampler2D DIFFUSE;

smooth in vec3 fNormal;
smooth in vec2 fUV;
smooth in vec2 fFace;
flat in vec4 fLight;

void main()
{
    vec3 lightDir = normalize(vec3(0.5f, -1.0f, 0.25f));
    float dLight = min(1.0f, max(0.0f, dot(-lightDir, fNormal))*0.8 + 0.6f); // Directional Sunlight

    float bilinearLight = (fFace.x * fFace.y * fLight.a +
        (1.0f - fFace.x) * (1.0f - fFace.y) * fLight.r +
        fFace.x * (1.0f - fFace.y) * fLight.g +
        (1.0f - fFace.x) * fFace.y * fLight.b) * 0.015625f;

    float aLight = smoothstep(0, 1, bilinearLight * 1.125f - 0.125f) * 0.75 + 0.25; // Ambient Light from occlusion
//    float aLight = max(0.0f, clamp(bilinearLight * 0.015625f, 0, 1) * 1.2f - 0.2f); // Ambient Light from occlusion

    vec4 diffColor = texture(DIFFUSE, fUV);
    if (diffColor.a < 0.5f)
        discard;

    outFragColor = dLight * aLight * diffColor.rgb;
} 
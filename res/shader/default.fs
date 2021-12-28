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
    float dLight = min(1.2f, max(0.0f, dot(-lightDir, fNormal)) * 1.2 + 0.5f); // Directional Sunlight

    float bilinearLight = (fFace.x * fFace.y * fLight.a +
        (1.0f - fFace.x) * (1.0f - fFace.y) * fLight.r +
        fFace.x * (1.0f - fFace.y) * fLight.g +
        (1.0f - fFace.x) * fFace.y * fLight.b) * 0.015625f;

    float aLight = smoothstep(0.0, 1.0, min(bilinearLight, 1.0)) * 0.75 + 0.25; // Ambient Light from occlusion
//    float aLight = max(0.0f, clamp(bilinearLight * 0.015625f, 0, 1) * 1.2f - 0.2f); // Ambient Light from occlusion

    vec4 diffColor = texture(DIFFUSE, fUV);
    if (diffColor.a < 0.5f)
        discard;

    outFragColor = dLight * aLight * diffColor.rgb;
} 
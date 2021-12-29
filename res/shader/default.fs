#version 330 core
out vec3 outFragColor;

uniform sampler2D DIFFUSE;
uniform float INV_RENDER_DIST;
uniform float TIME;
uniform vec3 PLAYER_POSITION;

smooth in vec3 fNormal;
smooth in vec2 fUV;
smooth in vec2 fFace;
smooth in vec3 fragPosition;
flat in vec4 fLight;

void main()
{
    vec4 diffColor = texture(DIFFUSE, fUV);
    if (diffColor.a < 0.5f)
        discard;

    vec3 lightDir = normalize(vec3(0.5f, -1.0f, 0.25f));

    float bilinearLight = (fFace.x * fFace.y * fLight.a +
        (1.0f - fFace.x) * (1.0f - fFace.y) * fLight.r +
        fFace.x * (1.0f - fFace.y) * fLight.g +
        (1.0f - fFace.x) * fFace.y * fLight.b
    ) * 0.015625f;

    vec3 lightDirection = vec3(sin(TIME * 0.125f), cos(TIME * 0.125f), 0.0f);
    float dLight = min(1.2f, max(0.0f, dot(-lightDirection, fNormal)) * 0.7f + 0.5f); // Directional Sunlight
    float aLight = smoothstep(0.0, 1.0, min(bilinearLight, 1.0f) * 0.8f + 0.2f); // Ambient Light from occlusion


    vec3 delta = (PLAYER_POSITION - fragPosition) * INV_RENDER_DIST;
    float fog = 1.0f - min(dot(delta, delta), 1.0f);
    outFragColor = mix(vec3(0.8, 0.95, 1.0), aLight*dLight * diffColor.rgb, fog);
} 
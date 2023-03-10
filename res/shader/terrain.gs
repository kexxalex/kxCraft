#version 450 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in uint vMode[];
in uint vTexture[];
in uvec4 vLight[];

uniform mat4 MVP;
uniform float TIME;
uniform vec3 EYE_POSITION;
uniform bool DISTANCE_CULLING;
uniform bool HUD;

out mat3 gTBN;
out vec2 gFace;
out vec2 gUV;
out vec3 gFragPosition;
out vec4 gLight;
out vec3 gLightDir;
flat out uint gTextureID;

const float WAVE_SPEED = 1.0f;
const float WAVE_STRENGTH = 0.15f;

void emit(in const vec2 uv) {
    const vec3 pos = gl_in[0].gl_Position.xyz;
    const vec3 view = pos - EYE_POSITION + gTBN[0] * 0.5 + gTBN[1] * 0.5;
    if (dot(gTBN[2], view) > 0.0)
        return;

    gFragPosition = pos;
    gl_Position = MVP * vec4(gFragPosition, 1.0);
    gUV = uv;
    gFace = vec2(0.0, 0.0);
    EmitVertex();

    gFragPosition = pos + gTBN[0];
    gl_Position = MVP * vec4(gFragPosition, 1.0);
    gUV = uv + vec2(0.0625, 0.0);
    gFace = vec2(1.0, 0.0);
    EmitVertex();
    
    gFragPosition = pos + gTBN[1];
    gl_Position = MVP * vec4(gFragPosition, 1.0);
    gUV = uv + vec2(0.0, 0.0625);
    gFace = vec2(0.0, 1.0);
    EmitVertex();

    gFragPosition = pos + gTBN[0] + gTBN[1];
    gl_Position = MVP * vec4(gFragPosition, 1.0);
    gUV = uv + vec2(0.0625, 0.0625);
    gFace = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}

void main() {
    gLightDir = normalize(vec3(1.0, -1.0, 0.5));

    gTextureID = vTexture[0];

    vec2 uv = 0.0625 * ivec2(vTexture[0] % 16, 15 - vTexture[0] / 16);
    gLight = vec4(vLight[0]) * (1.0 / 60.0);

    switch (vMode[0]) {
        case 0:
            gTBN = mat3(vec3(0, 0, 1), vec3(1, 0, 0), vec3(0, 1, 0));
            emit(uv);
            break;
        case 1:
            gTBN = mat3(vec3(1, 0, 0), vec3(0, 0, 1), vec3(0, -1, 0));
            emit(uv);
            break;
        case 2:
            gTBN = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1));
            emit(uv);
            break;
        case 3:
            gTBN = mat3(vec3(0, 0, -1), vec3(0, 1, 0), vec3(1, 0, 0));
            emit(uv);
            break;
        case 4:
            gTBN = mat3(vec3(-1, 0, 0), vec3(0, 1, 0), vec3(0, 0, -1));
            emit(uv);
            break;
        case 5:
            gTBN = mat3(vec3(0, 0, 1), vec3(0, 1, 0), vec3(-1, 0, 0));
            emit(uv);
            break;
    }
}

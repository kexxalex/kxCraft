#version 450 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

in int vTexture[];
in int vLight[];

uniform mat4 MVP;
uniform float TIME;
uniform vec3 PLAYER_POSITION;
uniform bool DISTANCE_CULLING;
uniform bool HUD;

out vec3 gNormal;
out vec2 gFace;
out vec2 gUV;
out vec3 gFragPosition;
out vec4 gLight;

const float WAVE_SPEED = 1.0f;
const float WAVE_STRENGTH = 0.15f;

void main() {
    vec3 pos = gl_in[0].gl_Position.xyz;
    vec3 delta = (PLAYER_POSITION - pos);
    if (DISTANCE_CULLING && vTexture[0] == 39 && dot(delta, delta) > 65536.0f)
        return;

    vec3 edgeA = gl_in[1].gl_Position.xyz - pos;
    vec3 edgeB = gl_in[2].gl_Position.xyz - pos;
    if (HUD) {
        pos -= 0.5f;
    }

    vec3 wave = vec3(0);
    if (vTexture[0] == 39) {
        gNormal = vec3(0, 1, 0);
        wave = vec3(sin(TIME * WAVE_SPEED + pos.x - 0.5 * pos.y), 0, sin(TIME * WAVE_SPEED - 0.5* pos.y + pos.z)) * WAVE_STRENGTH;
    }
    else {
        gNormal = normalize(cross(edgeA, edgeB));
        // Cull face of blocks
        if (dot(gNormal, delta) < 0)
            return;
    }

    vec2 uv = 0.0625f * vec2(vTexture[0] % 16, 15 - vTexture[0] / 16);
    gLight = vec4(vLight[0], vLight[1], vLight[2], vTexture[1]) * 0.01666666666f;

    gl_Position = MVP * vec4(pos, 1.0);
    gFragPosition = pos;
    gUV = uv;
    gFace = vec2(0, 0);
    EmitVertex();

    gl_Position = MVP * vec4(pos + edgeA, 1.0);
    gFragPosition = pos + edgeA;
    gUV = uv + vec2(0.0625f, 0.0f);
    gFace = vec2(1, 0);
    EmitVertex();

    if (vTexture[0] == 39)
        gl_Position = MVP * vec4(pos + normalize(edgeB + wave), 1.0);
    else
        gl_Position = MVP * vec4(pos + edgeB, 1.0);
    gFragPosition = pos + edgeB;
    gUV = uv + vec2(0.0f, 0.0625f);
    gFace = vec2(0, 1);
    EmitVertex();

    if (vTexture[0] == 39)
        gl_Position = MVP * vec4(pos + edgeA + normalize(edgeB + wave), 1.0);
    else
        gl_Position = MVP * vec4(pos + edgeA + edgeB, 1.0);
    gFragPosition = pos + edgeA + edgeB;
    gUV = uv + vec2(0.0625f, 0.0625f);
    gFace = vec2(1, 1);
    EmitVertex();

    EndPrimitive();
}

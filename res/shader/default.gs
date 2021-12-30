#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

in int vTexture[];
in int vLight[];

uniform mat4 MVP;

out vec3 gNormal;
out vec2 gFace;
out vec2 gUV;
out vec3 gFragPosition;
out vec4 gLight;

void main() {
    vec2 uv = 0.0625f * vec2(vTexture[0] % 16, 15 - vTexture[0] / 16);
    vec3 pos = gl_in[0].gl_Position.xyz;

    vec3 edgeA = gl_in[1].gl_Position.xyz - pos;
    vec3 edgeB = gl_in[2].gl_Position.xyz - pos;

    gNormal = normalize(cross(edgeA, edgeB));
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

    gl_Position = MVP * vec4(pos + edgeB, 1.0);
    gFragPosition = pos + edgeB;
    gUV = uv + vec2(0.0f, 0.0625f);
    gFace = vec2(0, 1);
    EmitVertex();

    gl_Position = MVP * vec4(pos + edgeA + edgeB, 1.0);
    gFragPosition = pos + edgeA + edgeB;
    gUV = uv + vec2(0.0625f, 0.0625f);
    gFace = vec2(1, 1);
    EmitVertex();

    EndPrimitive();
}

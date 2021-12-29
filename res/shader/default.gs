#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

in int gTexture[];
in int gLight[];

uniform mat4 MVP;

smooth out vec3 fNormal;
smooth out vec2 fFace;
smooth out vec2 fUV;
smooth out vec3 fragPosition;
flat out vec4 fLight;

void main() {
    vec2 uv = 0.0625f * vec2(gTexture[0] % 16, 15 - gTexture[0] / 16);
    vec3 pos = gl_in[0].gl_Position.xyz;

    vec3 edgeA = gl_in[1].gl_Position.xyz - pos;
    vec3 edgeB = gl_in[2].gl_Position.xyz - pos;

    fNormal = normalize(cross(edgeA, edgeB));
    fLight = vec4(gLight[0], gLight[1], gLight[2], gTexture[1]);

    gl_Position = MVP * vec4(pos, 1.0);
    fragPosition = pos;
    fUV = uv;
    fFace = vec2(0, 0);
    EmitVertex();

    gl_Position = MVP * vec4(pos + edgeA, 1.0);
    fragPosition = pos + edgeA;
    fUV = uv + vec2(0.0625f, 0.0f);
    fFace = vec2(1, 0);
    EmitVertex();

    gl_Position = MVP * vec4(pos + edgeB, 1.0);
    fragPosition = pos + edgeB;
    fUV = uv + vec2(0.0f, 0.0625f);
    fFace = vec2(0, 1);
    EmitVertex();

    gl_Position = MVP * vec4(pos + edgeA + edgeB, 1.0);
    fragPosition = pos + edgeA + edgeB;
    fUV = uv + vec2(0.0625f, 0.0625f);
    fFace = vec2(1, 1);
    EmitVertex();

    EndPrimitive();
}

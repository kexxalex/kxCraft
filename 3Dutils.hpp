//
// Created by kexx on 04.01.22.
//

#pragma once
#include <glm/glm.hpp>
#include <vector>

static constexpr double PI = 3.141592653589793;
static constexpr double PI2 = 3.141592653589793 * 2;

struct st_vertex {
    unsigned char position[3]{ 0, 0, 0};
    unsigned char ID{ 0 };
    short light{ 0 };

    st_vertex() = default;
    st_vertex(const glm::ivec3& pos, short texID, short light)
            : position{(unsigned char)pos.x, (unsigned char)pos.y, (unsigned char)pos.z}, ID(texID), light(light)
    {

    }
};

inline int MOD(int n, int m) {
    return n % m + m * (n < 0);
}

void addFace(short ID,
             const glm::ivec3 &pos, const glm::ivec3 &edgeA, const glm::ivec3 &edgeB,
             const short (&light)[4], std::vector<st_vertex> &vertices);


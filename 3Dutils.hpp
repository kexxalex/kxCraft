//
// Created by kexx on 04.01.22.
//

#pragma once
#include <glm/glm.hpp>
#include <vector>

static constexpr double PI = 3.141592653589793;
static constexpr double PI2 = 3.141592653589793 * 2;

enum FACE { TOP, BOTTOM, NORTH, EAST, SOUTH, WEST, DIAG_A, DIAG_B };

struct st_face {
    uint8_t position[4]{ 0, 0, 0, FACE::TOP };
    uint8_t light[4]{ 0, 0, 0, 0 };
    uint16_t ID{ 0 };

    constexpr st_face() = default;
    constexpr st_face(short texID, const glm::ivec3& pos, FACE mode, const uint8_t corner_lights[4])
        : position{(uint8_t)pos.x, (uint8_t)pos.y, (uint8_t)pos.z, mode}
        , light{corner_lights[0], corner_lights[1], corner_lights[2], corner_lights[3]}
        , ID(texID)
    {

    }
};

constexpr int MOD(int n, int m) {
    return n % m + m * (n < 0);
}
/*
 * WorldGenerator.h
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#pragma once

#include <libnoise/noise.h>


static constexpr int C_EXTEND = 16;
static constexpr int C_HEIGHT = 384;
static constexpr unsigned char MAX_SUN_LIGHT = 16;

struct st_block {
    short block{ 0 };
    unsigned char sunLight{ MAX_SUN_LIGHT };
    unsigned char torchLight{ 0 };

    [[nodiscard]] inline short getLight() const {
        return sunLight > torchLight ? sunLight : torchLight;
    }
};
static constexpr st_block AIR_BLOCK{ 0, 0, 0 };

inline int linearizeCoord(int x, int y, int z) {
    return (z * C_EXTEND + x) * C_HEIGHT + y;
}


class WorldGenerator {
public:
    WorldGenerator(int seed);

    void placeStack(int x, int z, st_block * stack) const;

private:
    noise::module::Perlin mountain_noise;
    noise::module::Perlin meadow_noise;
};

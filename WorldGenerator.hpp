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
static constexpr unsigned char MAX_SUN_LIGHT = 15;
static constexpr unsigned char NO_LIGHT = 0xFF;

struct st_block {
    short ID{ 0 };
    unsigned char sunLight{ 0 };
    unsigned char torchLight{ 0 };

    [[nodiscard]] inline unsigned char getSunLight() const noexcept { return (sunLight == NO_LIGHT) ? 0 : sunLight; }
    [[nodiscard]] inline short getLight() const noexcept {
        return (sunLight != NO_LIGHT && sunLight > torchLight) ? sunLight : torchLight;
    }
};
static st_block AIR_BLOCK{ 0, NO_LIGHT, 0 };

inline int linearizeCoord(int x, int y, int z) {
    return (z * C_EXTEND + x) * C_HEIGHT + y;
}


class WorldGenerator {
public:
    WorldGenerator() = default;
    explicit WorldGenerator(int seed);

    WorldGenerator& operator=(const WorldGenerator& wg);

    void placeStack(int x, int z, st_block * stack) const;

private:
    noise::module::Billow mountainNoise;
    noise::module::Perlin meadowNoise;
    noise::module::Perlin caveNoise;
};

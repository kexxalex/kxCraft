/*
 * WorldGenerator.h
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#pragma once

#include <libnoise/noise.h>
#include "Block.hpp"


static constexpr int C_EXTEND = 16;
static constexpr int C_HEIGHT = 384;
static constexpr unsigned char MAX_LIGHT = 15;
static constexpr unsigned char LIGHT_MASK = 0b00001111; // 15

struct st_block {
    short ID{ 0 };

    st_block(short ID=BLOCK_ID::AIR, unsigned char sunLight = 0, unsigned char torchLight = 0)
        : ID(ID), light(0)
    {
        light = ((torchLight & LIGHT_MASK) << 4) | (sunLight & LIGHT_MASK);
    }

    [[nodiscard]] inline unsigned char getTorchLight() const { return (light >> 4) & LIGHT_MASK; }
    [[nodiscard]] inline unsigned char getSunLight() const { return light & LIGHT_MASK; }
    [[nodiscard]] inline short getLight() const {
        return (getSunLight() > getTorchLight()) ? getSunLight() : getTorchLight();
    }

    inline void setTorchLight(unsigned char torchLight) {
        light = ((torchLight & LIGHT_MASK) << 4) | (light & LIGHT_MASK);
    }
    inline void setSunLight(unsigned char sunLight) {
        light = light & (LIGHT_MASK << 4) | (sunLight & LIGHT_MASK);
    }

private:
    unsigned char light{ 0 }; // 0bTTTTSSSS -- (LittleEndian) first 4 bit for sunlight, next 4 bit for torch light

};
static st_block AIR_BLOCK{ 0};

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
    noise::module::Perlin oreNoise;
};

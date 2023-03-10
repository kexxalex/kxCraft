/*
 * WorldGenerator.h
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#pragma once

#include <noise/noise.h>
#include <glm/glm.hpp>
#include "Block.hpp"


static constexpr int C_EXTEND = 16;
static constexpr int C_HEIGHT = 256;
static constexpr int MAX_LIGHT = 15;
static constexpr uint8_t LIGHT_MASK = 0b00001111; // 15

static constexpr int OAK_TREE_HEIGHT = 5;
static constexpr int OAK_TREE_RADIUS = 2;

inline double terrace(double val, int steps)
{
    const double terraceStep = 1.0 / steps;
    double trc = glm::floor(val * steps) * terraceStep;

    return trc + glm::smoothstep(0.0, terraceStep, 1.15 * (val - trc)) * terraceStep;
}

struct st_block {
    uint8_t ID{ 0 };

    constexpr st_block(uint8_t ID=BLOCK_ID::AIR, uint8_t sunLight = 0, uint8_t torchLight = 0)
        : ID(ID), light(((torchLight & LIGHT_MASK) << 4) | (sunLight & LIGHT_MASK))
    {
    }

    [[nodiscard]] constexpr uint8_t getTorchLight() const { return (light >> 4) & LIGHT_MASK; }
    [[nodiscard]] constexpr uint8_t getSunLight() const { return light & LIGHT_MASK; }
    [[nodiscard]] constexpr short getLight() const {
        return (getSunLight() > getTorchLight()) ? getSunLight() : getTorchLight();
    }

    inline void setTorchLight(uint8_t torchLight) {
        light = ((torchLight & LIGHT_MASK) << 4) | (light & LIGHT_MASK);
    }
    inline void setSunLight(uint8_t sunLight) {
        light = light & (LIGHT_MASK << 4) | (sunLight & LIGHT_MASK);
    }

private:
    uint8_t light{ 0 }; // 0bTTTTSSSS -- (LittleEndian) first 4 bit for sunlight, next 4 bit for torch light
};

static st_block AIR_BLOCK(0);

inline int linearizeCoord(int x, int y, int z) {
    return (z * C_EXTEND + x) * C_HEIGHT + y;
}


class WorldGenerator {
public:
    WorldGenerator() = default;
    explicit WorldGenerator(int seed);

    WorldGenerator& operator=(const WorldGenerator& wg);

    void placeStack(int x, int z, st_block stack[C_HEIGHT]) const;
    void placeOakTree(int x, int y, int z, st_block blocks[C_EXTEND][C_EXTEND][C_HEIGHT]) const;
    void generate(int cx, int cz, st_block blocks[C_EXTEND][C_EXTEND][C_HEIGHT]) const;

private:
    noise::module::Billow mountainNoise;
    noise::module::Perlin meadowNoise;
    noise::module::Perlin caveNoise;
    noise::module::Perlin oreNoise;
};

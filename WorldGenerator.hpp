/*
 * WorldGenerator.h
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#pragma once

#include <libnoise/noise.h>
#include <glm/glm.hpp>
#include "Block.hpp"


static constexpr int C_EXTEND = 16;
static constexpr int C_HEIGHT = 256;
static constexpr unsigned char MAX_LIGHT = 15;
static constexpr unsigned char LIGHT_MASK = 0b00001111; // 15

static constexpr int OAK_TREE_HEIGHT = 5;
static constexpr int OAK_TREE_RADIUS = 2;

inline double terrace(double val, int steps)
{
    const double terraceStep = 1.0 / steps;
    double trc = glm::floor(val * steps) * terraceStep;

    return trc + glm::smoothstep(0.0, terraceStep, 1.15 * (val - trc)) * terraceStep;
}

struct st_block {
    unsigned char ID{ 0 };

    st_block(unsigned char ID=BLOCK_ID::AIR, unsigned char sunLight = 0, unsigned char torchLight = 0)
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

static st_block AIR_BLOCK(0);

inline int linearizeCoord(int x, int y, int z) {
    return (z * C_EXTEND + x) * C_HEIGHT + y;
}


class WorldGenerator {
public:
    WorldGenerator() = default;
    explicit WorldGenerator(int seed);

    WorldGenerator& operator=(const WorldGenerator& wg);

    void placeStack(int x, int z, st_block *stack) const;
    void placeOakTree(int x, int y, int z, st_block *blocks) const;
    inline void generate(int cx, int cz, st_block *blocks) const {
        for (int z = 0; z < C_EXTEND; z++) {
            for (int x = 0; x < C_EXTEND; x++) {
                placeStack(x + cx * C_EXTEND, z + cz * C_EXTEND, &blocks[linearizeCoord(x, 0, z)]);
            }
        }

        for (int z = -OAK_TREE_RADIUS; z <= C_EXTEND+OAK_TREE_RADIUS; z++) {
            for (int x = -OAK_TREE_RADIUS; x <= C_EXTEND+OAK_TREE_RADIUS; x++) {
                double mNoise = mountainNoise.GetValue(x + cx * C_EXTEND, z + cz * C_EXTEND, 0.0);
                double mountains = terrace(
                        glm::clamp(mNoise * 1 + 0.6, 0.0, 1.0),
                        6);
                int height = static_cast<int>(C_HEIGHT / 3.0 * (1.0 + mountains)
                                              + 3 * meadowNoise.GetValue(x + cx * C_EXTEND, z + cz * C_EXTEND, 0.0)
                                              + 12 * mNoise
                );

                double ore = oreNoise.GetValue(x + cx * C_EXTEND, z + cz * C_EXTEND, 0.0);

                if (ore > 0.5 && ore < 0.505)
                    placeOakTree(x, height, z, blocks);
            }
        }
    }

private:
    noise::module::Billow mountainNoise;
    noise::module::Perlin meadowNoise;
    noise::module::Perlin caveNoise;
    noise::module::Perlin oreNoise;
};

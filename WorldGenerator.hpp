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
    unsigned char sunLight{ 0 };
    unsigned char torchLight{ 0 };

    [[nodiscard]] inline short getSunLight() const {
        return (sunLight == 255) ? 0 : sunLight;
    }

    [[nodiscard]] inline short getLight() const {
        return (sunLight != 255 && sunLight > torchLight) ? sunLight : torchLight;
    }
};
static st_block AIR_BLOCK{ 0, 255, 0 };

inline int linearizeCoord(int x, int y, int z) {
    return (z * C_EXTEND + x) * C_HEIGHT + y;
}


class WorldGenerator {
public:
    WorldGenerator() = default;
    explicit WorldGenerator(int seed);

    WorldGenerator& operator=(const WorldGenerator& wg) {
        mountainNoise.SetSeed(wg.mountainNoise.GetSeed());
        mountainNoise.SetOctaveCount(wg.mountainNoise.GetOctaveCount());
        mountainNoise.SetNoiseQuality(wg.mountainNoise.GetNoiseQuality());
        mountainNoise.SetFrequency(wg.mountainNoise.GetFrequency());

        meadowNoise.SetSeed(wg.meadowNoise.GetSeed());
        meadowNoise.SetOctaveCount(wg.meadowNoise.GetOctaveCount());
        meadowNoise.SetNoiseQuality(wg.meadowNoise.GetNoiseQuality());
        meadowNoise.SetFrequency(wg.meadowNoise.GetFrequency());

        caveNoise.SetSeed(wg.caveNoise.GetSeed());
        caveNoise.SetOctaveCount(wg.caveNoise.GetOctaveCount());
        caveNoise.SetNoiseQuality(wg.caveNoise.GetNoiseQuality());
        caveNoise.SetFrequency(wg.caveNoise.GetFrequency());

        return *this;
    }

    void placeStack(int x, int z, st_block * stack) const;

private:
    noise::module::Billow mountainNoise;
    noise::module::Perlin meadowNoise;
    noise::module::Perlin caveNoise;
};

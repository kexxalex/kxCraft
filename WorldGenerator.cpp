/*
 * WorldGenerator.cpp
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#include "WorldGenerator.hpp"
#include "Block.hpp"
#include <glm/glm.hpp>

inline double terrace(double val, int steps)
{
    const double terraceStep = 1.0 / steps;
    double trc = glm::floor(val * steps) * terraceStep;

    return trc + glm::smoothstep(0.0, terraceStep, 1.15 * (val - trc)) * terraceStep;
}

WorldGenerator::WorldGenerator(int seed) {
    mountainNoise.SetSeed(seed);
    mountainNoise.SetOctaveCount(10);
    mountainNoise.SetNoiseQuality(noise::NoiseQuality::QUALITY_BEST);
    mountainNoise.SetFrequency(1.0 / 1024.0);

    meadowNoise.SetSeed(seed + 2347);
    meadowNoise.SetOctaveCount(4);
    meadowNoise.SetNoiseQuality(noise::NoiseQuality::QUALITY_BEST);
    meadowNoise.SetFrequency(1.0 / 32.0);

    caveNoise.SetSeed(seed + 354897);
    caveNoise.SetOctaveCount(4);
    caveNoise.SetNoiseQuality(noise::NoiseQuality::QUALITY_BEST);
    caveNoise.SetFrequency(1.0 / 64.0);
}

WorldGenerator &WorldGenerator::operator=(const WorldGenerator &wg) {
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

void WorldGenerator::placeStack(int x, int z, st_block * stack) const {
    double mNoise = mountainNoise.GetValue(x, z, 0.0);
    double mountains = terrace(
            glm::clamp(mNoise * 1.2 + 0.3, 0.0, 1.0),
            6);
    int height = static_cast<int>(C_HEIGHT / 3.0 * (1.0 + mountains)
            + 3 * meadowNoise.GetValue(x, z, 0.0)
            + 12 * mNoise
    );

    for (int y = 0; y < height; y++) {
        if (caveNoise.GetValue(x, y * 1.5, z) < 0.8) {
            stack[y].ID = (y < height - 4) ? BLOCK_ID::STONE : BLOCK_ID::DIRT;
        }
        //stack[y].sunLight = 0;
    }

    if (caveNoise.GetValue(x, height * 1.5, z) < 0.8) {
        stack[height].ID = BLOCK_ID::GRASS;
        //stack[height].sunLight = 0;
    }
}


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
    mountain_noise.SetSeed(seed);
    mountain_noise.SetOctaveCount(6);
    mountain_noise.SetNoiseQuality(noise::NoiseQuality::QUALITY_BEST);
    mountain_noise.SetFrequency(1.0 / 1024.0);

    meadow_noise.SetSeed(seed + 2347);
    meadow_noise.SetOctaveCount(4);
    meadow_noise.SetNoiseQuality(noise::NoiseQuality::QUALITY_BEST);
    meadow_noise.SetFrequency(1.0 / 32.0);
}

void WorldGenerator::placeStack(int x, int z, st_block * stack) const {
    double mountains = terrace(
            glm::clamp(mountain_noise.GetValue(x, z, 0.0) * 0.8 + 0.2, 0.0, 1.0),
            8);
    int height = static_cast<int>(C_HEIGHT / 3.0 * (1.0 + mountains) + 3 * meadow_noise.GetValue(x, z, 0.0));

    st_block * const stack_start = stack;
    while ((stack - stack_start) < (height - 4)) {
        (*stack).block = BLOCK_ID::STONE;
        (*stack++).sunLight = 0;
    }

    while ((stack - stack_start) < height) {
        (*stack).block = BLOCK_ID::DIRT;
        (*stack++).sunLight = 0;
    }

    (*stack).block = BLOCK_ID::GRASS;
    (*stack).sunLight = 0;
    /*
    while ((stack - stack_start) < C_HEIGHT) {
        (*stack++).sunLight = MAX_SUN_LIGHT;
    } */
}


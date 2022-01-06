/*
 * WorldGenerator.cpp
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#include "WorldGenerator.hpp"
#include "Block.hpp"


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
    caveNoise.SetOctaveCount(5);
    caveNoise.SetNoiseQuality(noise::NoiseQuality::QUALITY_BEST);
    caveNoise.SetFrequency(1.0 / 48.0);

    oreNoise.SetSeed(seed - 2367);
    oreNoise.SetOctaveCount(5);
    oreNoise.SetNoiseQuality(noise::NoiseQuality::QUALITY_BEST);
    oreNoise.SetFrequency(1.0 / 16.0);
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

    oreNoise.SetSeed(wg.oreNoise.GetSeed());
    oreNoise.SetOctaveCount(wg.oreNoise.GetOctaveCount());
    oreNoise.SetNoiseQuality(wg.oreNoise.GetNoiseQuality());
    oreNoise.SetFrequency(wg.oreNoise.GetFrequency());

    return *this;
}

void WorldGenerator::placeStack(int x, int z, st_block *stack) const {
    double mNoise = mountainNoise.GetValue(x, z, 0.0);
    double mountains = terrace(
            glm::clamp(mNoise * 1 + 0.6, 0.0, 1.0),
            6);
    int height = static_cast<int>(C_HEIGHT / 3.0 * (1.0 + mountains)
                                  + 3 * meadowNoise.GetValue(x, z, 0.0)
                                  + 12 * mNoise
    );

    stack[0] = BLOCK_ID::BEDROCK;

    for (int y = 1; y < C_HEIGHT; y++) {
        if (y < height && abs(caveNoise.GetValue(x, y * 1.5, z)) < 0.9) {
            if (y >= height - 4)
                stack[y].ID = BLOCK_ID::DIRT;
            else {
                double on = oreNoise.GetValue(x, y, z);
                if (abs(on - 0.6) < 0.03 * (1 - abs(y - C_HEIGHT / 4.0) * 0.03125)) {
                    stack[y].ID = BLOCK_ID::COAL_ORE;
                } else if (abs(on - 0.4) < 0.03 * (1 - abs(y - C_HEIGHT / 5.0) * 0.03125)) {
                    stack[y].ID = BLOCK_ID::IRON_ORE;
                } else if (abs(on + 0.4) < 0.02 * (1 - abs(y - C_HEIGHT / 7.0) * 0.03125)) {
                    stack[y].ID = BLOCK_ID::GOLD_ORE;
                } else if (abs(on) < 0.007 * (1 - abs(y - 10) * 0.125)) {
                    stack[y].ID = BLOCK_ID::DIAMOND_ORE;
                } else {
                    stack[y].ID = BLOCK_ID::STONE;
                }
            }
        } else {
            stack[y].ID = BLOCK_ID::AIR;
        }
    }

    if (caveNoise.GetValue(x, height * 1.5, z) < 0.8) {
        stack[height].ID = BLOCK_ID::GRASS;

        double onVal = oreNoise.GetValue(x, height+1, z);
        if ((onVal > -0.1 && onVal < 0.1) || (abs(onVal) > 0.5 && abs(onVal) < 0.55)) {
            stack[height + 1].ID = BLOCK_ID::TALL_GRASS;
        } else if (onVal > 0.8 && onVal < 0.82) {
            stack[height + 1].ID = BLOCK_ID::TULIP;
        } else if (onVal > -0.83 && onVal < -0.8) {
            stack[height + 1].ID = BLOCK_ID::SUNFLOWER;
        } else if (abs(onVal) > 0.99 && abs(onVal) < 0.9975) {
            stack[height + 1].ID = BLOCK_ID::LILY;
        }
    }
}

void WorldGenerator::placeOakTree(int x, int y, int z, st_block *blocks) const {
    if (x >= 0 && y >= 0 && z >= 0 && x < C_EXTEND && z < C_EXTEND) {
        for (int dy = 0; dy < OAK_TREE_HEIGHT && y + dy < C_HEIGHT; dy++)
            blocks[linearizeCoord(x, y + dy, z)].ID = BLOCK_ID::OAK_LOG;
    }

    for (int dx = -OAK_TREE_RADIUS; dx <= OAK_TREE_RADIUS; dx++) {
        for (int dz = -OAK_TREE_RADIUS; dz <= OAK_TREE_RADIUS; dz++) {
            for (int dy = -OAK_TREE_RADIUS; dy <= OAK_TREE_RADIUS; dy++) {
                if (x+dx >= 0 && y+dy+OAK_TREE_HEIGHT >= 0 && z+dz >= 0
                    && x+dx < C_EXTEND && y+dy+OAK_TREE_HEIGHT < C_HEIGHT && z+dz < C_EXTEND
                    && abs(dx) + abs(dy) < 2*OAK_TREE_RADIUS && abs(dy) + abs(dz) < 2*OAK_TREE_RADIUS && abs(dx) + abs(dz) < 2*OAK_TREE_RADIUS)
                {
                    if (blocks[linearizeCoord(x+dx, y+OAK_TREE_HEIGHT+dy, z+dz)].ID == BLOCK_ID::AIR)
                        blocks[linearizeCoord(x+dx, y+OAK_TREE_HEIGHT+dy, z+dz)].ID = BLOCK_ID::OAK_LEAVES;
                }
            }
        }
    }
}

void WorldGenerator::generate(int cx, int cz, st_block *blocks) const {
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

            if (abs(0.5 - abs(ore)) > 0.498 && abs(mountainNoise.GetValue(z + cz * C_EXTEND, 0.0, x + cx * C_EXTEND)) > 0.75)
                placeOakTree(x, height, z, blocks);
        }
    }
}


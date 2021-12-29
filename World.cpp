/*
 * World.cpp
 *
 *  Created on: Dec 26, 2021
 *      Author: kexx
 */

#include "World.hpp"
#include <iostream>
#include <GLFW/glfw3.h>


World::~World() {
    chunks.clear();
}

World::World(const glm::fvec3 &start_position, int seed, int renderDistance, int threadCount)
        : renderDistance(renderDistance), threadCount(threadCount), worldGenerator(seed), chunks(4 * renderDistance * renderDistance) {
    chunks.reserve((2*renderDistance+1) * (2*renderDistance+1));
}

void World::update(int threadID) {
    glm::fvec3 position = playerPosition;
    glm::fvec3 direction = playerDirection;

    glm::ivec2 chunkPos = glm::ivec2(
            glm::floor(position.x / C_EXTEND),
            glm::floor(position.z / C_EXTEND)
    ) * C_EXTEND;

    if (threadID == 0)
        addChunk(chunkPos);

    // add rectangular edge chunks with 'radius' r
    /*
                   +

                   |
             +  <--+-->   +
                   |

                   +
    */
    for (int r = 1 + threadID; r <= renderDistance && glm::dot(direction, playerDirection) > 0.98f; r += threadCount) {
        // center of each edge
        addChunk({chunkPos.x + r * C_EXTEND, chunkPos.y});
        addChunk({chunkPos.x - r * C_EXTEND, chunkPos.y});
        addChunk({chunkPos.x, chunkPos.y + r * C_EXTEND});
        addChunk({chunkPos.x, chunkPos.y - r * C_EXTEND});

        // for each edge from its center to its corner
        /*
                +   <--+-->   +
                       |
                |      |      |
                +------+------+
                |      |      |
                       |
                +   <--+-->   +
        */
        for (int s = 1; s <= r && glm::dot(direction, playerDirection) > 0.98f; s++) {
            addChunk({chunkPos.x + r * C_EXTEND, chunkPos.y + s * C_EXTEND});
            addChunk({chunkPos.x + r * C_EXTEND, chunkPos.y - s * C_EXTEND});

            addChunk({chunkPos.x - r * C_EXTEND, chunkPos.y + s * C_EXTEND});
            addChunk({chunkPos.x - r * C_EXTEND, chunkPos.y - s * C_EXTEND});

            if (s < r) {
                addChunk({chunkPos.x + s * C_EXTEND, chunkPos.y + r * C_EXTEND});
                addChunk({chunkPos.x - s * C_EXTEND, chunkPos.y + r * C_EXTEND});

                addChunk({chunkPos.x + s * C_EXTEND, chunkPos.y - r * C_EXTEND});
                addChunk({chunkPos.x - s * C_EXTEND, chunkPos.y - r * C_EXTEND});
            }
        }
    }

    for (auto& chunk : chunks) {
        if (glm::dot(direction, playerDirection) < 0.95)
            break;

        Chunk &c = chunk.second;
        const glm::ivec2 north = c.getXZPosition() + glm::ivec2(0, C_EXTEND);
        const glm::ivec2 east = c.getXZPosition() + glm::ivec2(C_EXTEND, 0);
        const glm::ivec2 south = c.getXZPosition() + glm::ivec2(0, -C_EXTEND);
        const glm::ivec2 west = c.getXZPosition() + glm::ivec2(-C_EXTEND, 0);

        if (chunkLock.try_lock()) {
            if (c.getNorth() == nullptr && chunks.find(north) != chunks.end())
                c.setNorth(&chunks[north]);
            if (c.getEast() == nullptr && chunks.find(east) != chunks.end())
                c.setEast(&chunks[east]);
            if (c.getSouth() == nullptr && chunks.find(south) != chunks.end())
                c.setSouth(&chunks[south]);
            if (c.getWest() == nullptr && chunks.find(west) != chunks.end())
                c.setWest(&chunks[west]);
            chunkLock.unlock();
        }

        if (c.needUpdate()) {
            c.update();
        }
    }
}

void World::addChunk(const glm::ivec2 &position) {
    glm::ivec2 chunkPos = glm::ivec2(
            glm::floor(playerPosition.x / C_EXTEND),
            glm::floor(playerPosition.z / C_EXTEND)
    ) * C_EXTEND;

    auto delta = glm::fvec2(position - chunkPos) / (float)C_EXTEND;
    if (glm::dot(delta, delta) > static_cast<float>(renderDistance*renderDistance))
        return;

    glm::fvec2 sideA = glm::fvec2(playerDirection.x, playerDirection.z);
    glm::fvec2 sideB = glm::fvec2(position - chunkPos)
            + glm::fvec2(C_EXTEND * 0.5) + 3.0f * sideA * (float) C_EXTEND;

    float angle = glm::dot(sideA, sideB);
    if (angle > glm::cos(45.0f) * glm::length(sideA) * glm::length(sideB)
        && chunks.find(position) == chunks.end())
    {
        chunkLock.lock();
        chunks[position] = Chunk(&worldGenerator, position.x, position.y);
        chunkLock.unlock();

        chunks[position].generate();
        chunks[position].update();
    }
}

void World::render() {
    glm::ivec2 chunkPos = glm::ivec2(
            glm::floor(playerPosition.x / C_EXTEND),
            glm::floor(playerPosition.z / C_EXTEND)
    ) * C_EXTEND;

    static std::vector<glm::ivec2> rmChunks(CHANGE_CHUNK_MAX);
    rmChunks.clear();
    int changes = 0;

    chunkLock.lock();
    for (auto &chunk: chunks) {
        auto delta = glm::fvec2(chunk.first - chunkPos) / (float)C_EXTEND;
        bool outOfRange = glm::dot(delta, delta) > static_cast<float>(renderDistance*renderDistance) + 1;
        if (chunk.second.isGenerated() && rmChunks.size() < CHANGE_CHUNK_MAX && outOfRange) {
            rmChunks.push_back(chunk.first);
        }

        if (outOfRange)
            continue;

        glm::fvec2 sideA = glm::fvec2(playerDirection.x, playerDirection.z);
        glm::fvec2 sideB =
                glm::fvec2(chunk.first - chunkPos) + glm::fvec2(C_EXTEND * 0.5) + 2.0f * sideA * (float) C_EXTEND;
        float angle = glm::dot(sideA, sideB);

        if (angle > glm::cos(45.0f) * glm::length(sideA) * glm::length(sideB)) { // || angle < glm::cos(65.0f * 0.5f))
            chunk.second.render();
        }
    }

    for (glm::ivec2 &rmChunk: rmChunks) {
        chunks.erase(rmChunk);
    }
    chunkLock.unlock();
}

void World::reloadCurrentChunk() {
    const glm::ivec2 chunkPos = glm::ivec2(
            glm::floor(playerPosition.x / C_EXTEND),
            glm::floor(playerPosition.z / C_EXTEND)
    ) * C_EXTEND;
    if (chunks.find(chunkPos) != chunks.end())
        chunks[chunkPos].update();
}

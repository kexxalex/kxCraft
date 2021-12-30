/*
 * World.cpp
 *
 *  Created on: Dec 26, 2021
 *      Author: kexx
 */

#include "World.hpp"
#include <iostream>
#include <GLFW/glfw3.h>


static const float CLIP_ANGLE = glm::cos(glm::radians(40.0f));


World::~World() {
    cleanUp();
}

World::World(const glm::fvec3 &start_position, int seed, int renderDistance, int threadCount)
        : renderDistance(renderDistance), threadCount(threadCount), worldGenerator(seed), chunks(4 * renderDistance * renderDistance), playerPosition(start_position)
{

}

void World::cleanUp() {
    chunks.clear();
}

const st_block &World::getBlock(float x, float y, float z) const {
    const glm::ivec2 chunkPos = glm::ivec2(
            glm::floor(x / C_EXTEND),
            glm::floor(z / C_EXTEND)
    ) * C_EXTEND;
    glm::ivec2 inner(glm::floor(x - chunkPos.x), glm::floor(z - chunkPos.y));
    return (y < 0 || y >= C_HEIGHT || chunks.find(chunkPos) == chunks.end()) ?
           AIR_BLOCK : chunks.at(chunkPos).getBlock(inner.x, (int) y, inner.y);
}

short World::setBlock(float x, float y, float z, short ID, bool forceChunkUpdate) {
    const glm::ivec2 chunkPos = glm::ivec2(
            glm::floor(x / C_EXTEND),
            glm::floor(z / C_EXTEND)
    ) * C_EXTEND;
    glm::ivec2 inner(glm::floor(x - chunkPos.x), glm::floor(z - chunkPos.y));

    return (y < 0 || y >= C_HEIGHT || chunks.find(chunkPos) == chunks.end()) ?
           (short)AIR : chunks.at(chunkPos).setBlock(inner.x, (int) y, inner.y, ID, forceChunkUpdate);
}

void World::update(int threadID) {
    glm::fvec3 position = playerPosition;
    glm::fvec3 direction = playerDirection;

    glm::ivec2 playerChunkPosition = glm::ivec2(
            glm::floor(position.x / C_EXTEND),
            glm::floor(position.z / C_EXTEND)
    ) * C_EXTEND;

    if (threadID == 0)
        updateChunk(playerChunkPosition);

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
        updateChunk({playerChunkPosition.x + r * C_EXTEND, playerChunkPosition.y});
        updateChunk({playerChunkPosition.x - r * C_EXTEND, playerChunkPosition.y});
        updateChunk({playerChunkPosition.x, playerChunkPosition.y + r * C_EXTEND});
        updateChunk({playerChunkPosition.x, playerChunkPosition.y - r * C_EXTEND});

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
            updateChunk({playerChunkPosition.x + r * C_EXTEND, playerChunkPosition.y + s * C_EXTEND});
            updateChunk({playerChunkPosition.x + r * C_EXTEND, playerChunkPosition.y - s * C_EXTEND});

            updateChunk({playerChunkPosition.x - r * C_EXTEND, playerChunkPosition.y + s * C_EXTEND});
            updateChunk({playerChunkPosition.x - r * C_EXTEND, playerChunkPosition.y - s * C_EXTEND});

            if (s < r) {
                updateChunk({playerChunkPosition.x + s * C_EXTEND, playerChunkPosition.y + r * C_EXTEND});
                updateChunk({playerChunkPosition.x - s * C_EXTEND, playerChunkPosition.y + r * C_EXTEND});

                updateChunk({playerChunkPosition.x + s * C_EXTEND, playerChunkPosition.y - r * C_EXTEND});
                updateChunk({playerChunkPosition.x - s * C_EXTEND, playerChunkPosition.y - r * C_EXTEND});
            }
        }
    }
}

void World::updateChunk(const glm::ivec2 &position) {
    glm::ivec2 chunkPos = glm::ivec2(
            glm::floor(playerPosition.x / C_EXTEND),
            glm::floor(playerPosition.z / C_EXTEND)
    ) * C_EXTEND;

    auto delta = glm::fvec2(position - chunkPos) / (float)C_EXTEND;
    if (glm::dot(delta, delta) > static_cast<float>(renderDistance*renderDistance))
        return;

    glm::fvec2 sideA = glm::normalize(glm::fvec2(playerDirection.x, playerDirection.z));
    glm::fvec2 sideB = glm::normalize(glm::fvec2(position - chunkPos) + glm::fvec2(C_EXTEND * 0.5) + 4.0f * sideA * (float) C_EXTEND);
    float angle = glm::dot(sideA, sideB);


    if (chunks.find(position) == chunks.end())
    {
        chunkLock.lock();
        chunks[position] = Chunk(&worldGenerator, position.x, position.y);
        chunkLock.unlock();

        chunks.at(position).generate();
    }

    Chunk& c = chunks.at(position);

    if (chunkLock.try_lock()) {
        const glm::ivec2 north = position + glm::ivec2(0, C_EXTEND);
        const glm::ivec2 east = position + glm::ivec2(C_EXTEND, 0);
        const glm::ivec2 south = position + glm::ivec2(0, -C_EXTEND);
        const glm::ivec2 west = position + glm::ivec2(-C_EXTEND, 0);

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

    if (angle > CLIP_ANGLE && c.needUpdate()) {
        chunks.at(position).update();
    }
}

void World::render(){
    glm::ivec2 chunkPos = glm::ivec2(
            glm::floor(playerPosition.x / C_EXTEND),
            glm::floor(playerPosition.z / C_EXTEND)
    ) * C_EXTEND;

    static std::vector<glm::ivec2> rmChunks(CHANGE_CHUNK_MAX);
    rmChunks.clear();
    int availableChanges = CHANGE_CHUNK_MAX;

    for (auto &chunk: chunks) {
        auto delta = glm::fvec2(chunk.first - chunkPos) / (float)C_EXTEND;
        bool outOfRange = glm::dot(delta, delta) > static_cast<float>(renderDistance*renderDistance) + 1;
        if (chunk.second.isGenerated() && rmChunks.size() < CHANGE_CHUNK_MAX && outOfRange) {
            rmChunks.push_back(chunk.first);
        }

        if (outOfRange)
            continue;

        glm::fvec2 sideA = glm::normalize(glm::fvec2(playerDirection.x, playerDirection.z));
        glm::fvec2 sideB = glm::normalize(glm::fvec2(chunk.first - chunkPos) + glm::fvec2(C_EXTEND * 0.5) + 4.0f * sideA * (float) C_EXTEND);
        float angle = glm::dot(sideA, sideB);

        if (angle > CLIP_ANGLE) { // || angle < glm::cos(65.0f * 0.5f))
            chunk.second.render(availableChanges);
        }
    }

    if (chunkLock.try_lock()) {
        for (glm::ivec2 &rmChunk: rmChunks) {
            chunks.erase(rmChunk);
        }
        chunkLock.unlock();
    }
}

void World::reloadCurrentChunk() {
    const glm::ivec2 chunkPos = glm::ivec2(
            glm::floor(playerPosition.x / C_EXTEND),
            glm::floor(playerPosition.z / C_EXTEND)
    ) * C_EXTEND;
    if (chunks.find(chunkPos) != chunks.end())
        chunks[chunkPos].update();
}

/*
 * World.cpp
 *
 *  Created on: Dec 26, 2021
 *      Author: kexx
 */

#include "World.hpp"
#include <GLFW/glfw3.h>
#include <iostream>


static const float CLIP_ANGLE = glm::cos(glm::radians(45.0f));
constexpr glm::ivec2 tJunctionOffset[4] = {
        {1,1},
        {1,-1},
        {-1,-1},
        {-1,1}
};
constexpr glm::ivec2 tOffset[4] = {
        {0,1},
        {0,-1},
        {1,0},
        {-1,0}
};


World::~World() {
    cleanUp();
}

World::World(const glm::fvec3 &start_position, int seed, int renderDistance, int threadCount)
        : renderDistance(renderDistance), threadCount(threadCount), worldGenerator(seed),
          chunks(4 * renderDistance * renderDistance), playerPosition(start_position)
{

}

void World::cleanUp() {
    chunks.clear();
}

bool World::toChunkPosition(float x, float y, float z, glm::ivec2 &chunkPos) const {
    chunkPos = glm::ivec2( glm::floor(x / C_EXTEND), glm::floor(z / C_EXTEND) ) * C_EXTEND;
    return y >= 0 && y < C_HEIGHT && (chunks.find(chunkPos) != chunks.end());
}

bool World::toChunkPositionAndOffset(float x, float y, float z, glm::ivec2 &chunkPos, glm::ivec2 &inner) const {
    chunkPos = glm::ivec2( glm::floor(x / C_EXTEND), glm::floor(z / C_EXTEND) ) * C_EXTEND;
    inner = glm::ivec2(glm::floor(x - (float)chunkPos.x), glm::floor(z - (float)chunkPos.y));
    return y >= 0 && y < C_HEIGHT && (chunks.find(chunkPos) != chunks.end());
}

const st_block &World::getBlock(float x, float y, float z) const {
    glm::ivec2 chunkPos, inner;
    return (toChunkPositionAndOffset(x, y, z, chunkPos, inner)
            ? chunks.at(chunkPos).getBlock(inner.x, (int) y, inner.y)
            : AIR_BLOCK);
}

short World::setBlock(float x, float y, float z, short ID, bool forceChunkUpdate) {
    glm::ivec2 chunkPos, inner;
    return (toChunkPositionAndOffset(x, y, z, chunkPos, inner)
            ? chunks.at(chunkPos).setBlock(inner.x, (int) y, inner.y, ID, forceChunkUpdate)
            : (short)AIR);
}

void World::update(int threadID) {
    glm::fvec3 direction = playerDirection;

    glm::ivec2 playerChunkPosition;
    toChunkPosition(playerPosition, playerChunkPosition);

    if (threadID == 0)
        updateChunk(playerChunkPosition);

    for (int r = 1 + threadID; r <= renderDistance && glm::dot(direction, playerDirection) > 0.98f; r += threadCount) {
        for (const glm::ivec2& offset : tOffset) {
            updateChunk(playerChunkPosition + r * offset * C_EXTEND);
        }

        for (int s = 1; s <= r && glm::dot(direction, playerDirection) > 0.98f; s++) {
            for (const glm::ivec2& offset : tJunctionOffset) {
                updateChunk({playerChunkPosition.x + r * offset.x * C_EXTEND,
                             playerChunkPosition.y + s * offset.y * C_EXTEND});
                if (s < r) {
                    updateChunk({playerChunkPosition.x + s * offset.x * C_EXTEND,
                                 playerChunkPosition.y + r * offset.y * C_EXTEND});
                }
            }

        }
    }
}

void World::updateChunkNeighbours(Chunk &chunk) {
    if (chunkLock.try_lock()) {
        const glm::ivec2& position = chunk.getXZPosition();
        const glm::ivec2 north = position + glm::ivec2(0, C_EXTEND);
        const glm::ivec2 east = position + glm::ivec2(C_EXTEND, 0);
        const glm::ivec2 south = position + glm::ivec2(0, -C_EXTEND);
        const glm::ivec2 west = position + glm::ivec2(-C_EXTEND, 0);

        if (chunk.getNorth() == nullptr && chunks.find(north) != chunks.end())
            chunk.setNorth(&chunks[north]);
        if (chunk.getEast() == nullptr && chunks.find(east) != chunks.end())
            chunk.setEast(&chunks[east]);
        if (chunk.getSouth() == nullptr && chunks.find(south) != chunks.end())
            chunk.setSouth(&chunks[south]);
        if (chunk.getWest() == nullptr && chunks.find(west) != chunks.end())
            chunk.setWest(&chunks[west]);
        chunkLock.unlock();
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

    // Generate the chunk at <position> if it doesn't exist
    if (chunks.find(position) == chunks.end())
    {
        chunkLock.lock();
        chunks[position] = Chunk(&worldGenerator, position.x, position.y);
        chunkLock.unlock();

        chunks.at(position).generate();
    }

    // Now, the chunk at <position> exists
    Chunk& chunk = chunks.at(position);
    updateChunkNeighbours(chunk);

    if (chunk.needUpdate()) {
        chunk.update();
    }
}

void World::forcePushBuffer() {
    for (auto &chunk: chunks)
        chunk.second.chunkBufferUpdate();
}

void World::render(Shader &shader){
    glm::ivec2 playerChunkPosition;
    toChunkPosition(playerPosition, playerChunkPosition);

    static std::vector<glm::ivec2> rmChunks(CHANGE_CHUNK_MAX);
    rmChunks.clear();
    int availableChanges = CHANGE_CHUNK_MAX;

    double t0 = glfwGetTime();
    for (auto &chunk: chunks) {
        auto deltaPlayerChunk = glm::fvec2(chunk.first - playerChunkPosition) / (float)C_EXTEND;
        bool outOfRange = glm::dot(deltaPlayerChunk, deltaPlayerChunk) > static_cast<float>(renderDistance*renderDistance) + 1;
        if (chunk.second.isGenerated() && rmChunks.size() < CHANGE_CHUNK_MAX && outOfRange) {
            rmChunks.push_back(chunk.first);
        }

        if (outOfRange)
            continue;

        glm::fvec2 sideA = glm::normalize(glm::fvec2(playerDirection.x, playerDirection.z));
        glm::fvec2 sideB = glm::normalize(glm::fvec2(chunk.first - playerChunkPosition) + glm::fvec2(C_EXTEND * 0.5) + 4.0f * sideA * (float) C_EXTEND);
        float angle = glm::dot(sideA, sideB);

        if (angle > CLIP_ANGLE) { // || angle < glm::cos(65.0f * 0.5f))
            chunk.second.render(availableChanges, shader);
        }
    }
    std::cout << (glfwGetTime() - t0) * 1000.0 << std::endl;

    if (chunkLock.try_lock()) {
        for (glm::ivec2 &rmChunk: rmChunks) {
            chunks.erase(rmChunk);
        }
        chunkLock.unlock();
    }
}

void World::reloadCurrentChunk() {
    glm::ivec2 playerChunkPosition;
    if (toChunkPosition(playerPosition, playerChunkPosition))
        chunks[playerChunkPosition].update();
}

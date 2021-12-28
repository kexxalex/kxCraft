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

void World::update(int threadID, glm::fvec3 *position_ptr, glm::fvec3 *direction_ptr) {
    glm::fvec3 position = *position_ptr;
    glm::fvec3 direction = *direction_ptr;

    glm::ivec2 chunkPos = glm::ivec2(
            glm::floor(position.x / C_EXTEND),
            glm::floor(position.z / C_EXTEND)
    ) * C_EXTEND;

    if (threadID == 0)
        addChunk(chunkPos, chunkPos, direction);
    for (int r = 1 + threadID; r <= renderDistance && glm::dot(direction, *direction_ptr) > 0.8; r += threadCount)
    {
        // add rectangular edge chunks with 'radius' r
        addChunk(chunkPos, {chunkPos.x + r * C_EXTEND, chunkPos.y}, direction);
        addChunk(chunkPos, {chunkPos.x - r * C_EXTEND, chunkPos.y}, direction);
        addChunk(chunkPos, {chunkPos.x, chunkPos.y + r * C_EXTEND}, direction);
        addChunk(chunkPos, {chunkPos.x, chunkPos.y - r * C_EXTEND}, direction);
        for (int s = 1; s <= r && glm::dot(direction, *direction_ptr) > 0.95; s++) {
            addChunk(chunkPos, {chunkPos.x + r * C_EXTEND, chunkPos.y + s * C_EXTEND}, direction);
            addChunk(chunkPos, {chunkPos.x + r * C_EXTEND, chunkPos.y - s * C_EXTEND}, direction);

            addChunk(chunkPos, {chunkPos.x - r * C_EXTEND, chunkPos.y + s * C_EXTEND}, direction);
            addChunk(chunkPos, {chunkPos.x - r * C_EXTEND, chunkPos.y - s * C_EXTEND}, direction);

            addChunk(chunkPos, {chunkPos.x + s * C_EXTEND, chunkPos.y + r * C_EXTEND}, direction);
            addChunk(chunkPos, {chunkPos.x - s * C_EXTEND, chunkPos.y + r * C_EXTEND}, direction);

            addChunk(chunkPos, {chunkPos.x + s * C_EXTEND, chunkPos.y - r * C_EXTEND}, direction);
            addChunk(chunkPos, {chunkPos.x - s * C_EXTEND, chunkPos.y - r * C_EXTEND}, direction);
        }
    }

    int counter = 0;

    for (auto &chunk: chunks) {
        if (counter++ % threadCount == threadID) {
            Chunk &c = chunk.second;
            const glm::ivec2 north = c.getPosition() + glm::ivec2(0, C_EXTEND);
            const glm::ivec2 east = c.getPosition() + glm::ivec2(C_EXTEND, 0);
            const glm::ivec2 south = c.getPosition() + glm::ivec2(0, -C_EXTEND);
            const glm::ivec2 west = c.getPosition() + glm::ivec2(-C_EXTEND, 0);

            bool hasChanges = false;

            if (c.north == nullptr && chunks.find(north) != chunks.end()) {
                c.north = &chunks[north];
                chunks[north].south = &c;
                chunks[north].m_needUpdate = true;

                hasChanges = true;
            }
            if (c.east == nullptr && chunks.find(east) != chunks.end()) {
                c.east = &chunks[east];
                chunks[east].west = &c;
                chunks[east].m_needUpdate = true;

                hasChanges = true;
            }
            if (c.south == nullptr && chunks.find(south) != chunks.end()) {
                c.south = &chunks[south];
                chunks[south].north = &c;
                chunks[south].m_needUpdate = true;

                hasChanges = true;
            }
            if (c.west == nullptr && chunks.find(west) != chunks.end()) {
                c.west = &chunks[west];
                chunks[west].east = &c;
                chunks[west].m_needUpdate = true;

                hasChanges = true;
            }

            if (hasChanges || c.needUpdate())
                c.update();
        }
    }
}

void World::render(const glm::fvec3 &position, const glm::fvec3 &direction) {
    glm::ivec2 chunkPos = glm::ivec2(
            glm::floor(position.x / C_EXTEND),
            glm::floor(position.z / C_EXTEND)
    ) * C_EXTEND;

    static std::vector<glm::ivec2> rmChunks(CHANGE_CHUNK_MAX);
    rmChunks.clear();

    chunkLock.lock();
    for (auto &chunk: chunks) {
        if (chunk.second.isGenerated() && rmChunks.size() < CHANGE_CHUNK_MAX &&
            (abs(chunk.first.x - chunkPos.x) - 1 > renderDistance * C_EXTEND ||
             abs(chunk.first.y - chunkPos.y) - 1 > renderDistance * C_EXTEND)) {
            rmChunks.push_back(chunk.first);
        }

        glm::fvec2 sideA = glm::fvec2(direction.x, direction.z);
        glm::fvec2 sideB = glm::fvec2(chunk.first - chunkPos) + glm::fvec2(C_EXTEND * 0.5) + 2.0f * sideA * (float) C_EXTEND;
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

void World::addChunk(const glm::ivec2 &chunkPos, const glm::ivec2 &position, const glm::fvec3 &direction) {
    glm::fvec2 sideA = glm::fvec2(direction.x, direction.z);
    glm::fvec2 sideB = glm::fvec2(position - chunkPos) + glm::fvec2(C_EXTEND * 0.5) + 2.0f * sideA * (float) C_EXTEND;
    float angle = glm::dot(sideA, sideB);

    if (angle > glm::cos(45.0f) * glm::length(sideA) * glm::length(sideB)
        && chunks.find(position) == chunks.end())
    {
        chunkLock.lock();
        chunks[position] = Chunk(&worldGenerator, position.x, position.y);
        chunkLock.unlock();

        chunks[position].generate();
    }
}

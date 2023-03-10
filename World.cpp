/*
 * World.cpp
 *
 *  Created on: Dec 26, 2021
 *      Author: kexx
 */

#include "World.hpp"
#include <iostream>
#include <iomanip>

static const float CLIP_ANGLE = glm::cos(glm::radians(FOV));
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



World::World(const glm::fvec3 &start_position, int seed, int renderDistance, int threadCount)
    : renderDistance(renderDistance), threadCount(threadCount), worldGenerator(seed), playerPosition(start_position),
      maxWorldExtend(2 * renderDistance + 3), chunks(new Chunk[maxWorldExtend * maxWorldExtend])
{
    initializeVertexArray();
    for (int dx = -renderDistance - 1; dx <= renderDistance + 1; dx++) {
        for (int dz = -renderDistance - 1; dz <= renderDistance + 1; dz++) {
            glm::ivec2 position(glm::ivec2(dx, dz));
            int linIndex = linearizeChunkPos(position);

            Chunk &north = chunks[linearizeChunkPos(position + glm::ivec2(0, 1))];
            Chunk &east = chunks[linearizeChunkPos(position + glm::ivec2(1, 0))];
            Chunk &south = chunks[linearizeChunkPos(position + glm::ivec2(0, -1))];
            Chunk &west = chunks[linearizeChunkPos(position + glm::ivec2(-1, 0))];

            chunks[linIndex].initialize(&worldGenerator, vboID, oboID, linIndex, &north, &east, &south, &west);
        }
    }

    generateChunk(toChunkPosition(playerPosition), -1);
}

World::~World() {
    delete[] chunks;

    glDeleteVertexArrays(1, &vaoID);
    glDeleteBuffers(1, &vboID);
    glDeleteBuffers(1, &oboID);
}


short World::setBlock(float x, float y, float z, short ID, bool forceChunkUpdate) {
    glm::ivec2 chunkPos, inner;
    toChunkPositionAndOffset(x, z, chunkPos, inner);
    return chunks[linearizeChunkPos(chunkPos)].setBlock(inner.x, (int) y, inner.y, ID, forceChunkUpdate);
}

void World::updateChunk(const glm::ivec2 &position, const glm::ivec2 &playerChunkPosition, int threadID) {
    int linIndex = linearizeChunkPos(position);
    if (threadID >= 0 && linIndex % threadCount != threadID)
        return;

    Chunk &chunk = chunks[linIndex];
    glm::fvec2 sideA = glm::normalize(glm::fvec2(playerDirection.x, playerDirection.z));
    glm::fvec2 sideB = glm::normalize(
            glm::fvec2(chunk.getXZPosition() - playerChunkPosition) + 0.5f + 2.0f * sideA);
    float angle = glm::dot(sideA, sideB);
    if (angle > CLIP_ANGLE && chunk.needUpdate())
        chunk.update();
    if (chunk.needBufferUpdate())
        hasChunkBufferChanges = true;
}

void World::update(int threadID) {
    glm::ivec2 playerChunkPosition = toChunkPosition(playerPosition);
    glm::fvec3 oldPlayerDirection = playerDirection;

    generateChunk(playerChunkPosition, threadID);
    updateChunk(playerChunkPosition, playerChunkPosition, threadID);
    for (int r = 1; r <= renderDistance && glm::dot(playerDirection, oldPlayerDirection) > 0.98; r++) {
        for (const glm::ivec2& offset : tOffset) {
            generateChunk(playerChunkPosition + r * offset, threadID);
            updateChunk(playerChunkPosition + r * offset, playerChunkPosition, threadID);
        }

        for (int s = 1; s < r && glm::dot(playerDirection, oldPlayerDirection) > 0.98; s++) {
            for (const glm::ivec2& offset : tJunctionOffset) {
                generateChunk({playerChunkPosition.x + r * offset.x,
                             playerChunkPosition.y + s * offset.y}, threadID);
                updateChunk({playerChunkPosition.x + r * offset.x,
                               playerChunkPosition.y + s * offset.y}, playerChunkPosition, threadID);
                generateChunk({playerChunkPosition.x + s * offset.x,
                             playerChunkPosition.y + r * offset.y}, threadID);
                updateChunk({playerChunkPosition.x + s * offset.x,
                               playerChunkPosition.y + r * offset.y}, playerChunkPosition, threadID);
            }
        }
        for (const glm::ivec2& offset : tJunctionOffset) {
            generateChunk(playerChunkPosition + r * offset, threadID);
            updateChunk(playerChunkPosition + r * offset, playerChunkPosition, threadID);
        }
    }
}

void World::render() const {
    glBindVertexArray(vaoID);
    glm::ivec2 playerChunkPosition = toChunkPosition(playerPosition);
    for (int i = 0; i < maxWorldExtend * maxWorldExtend; ++i) {
        const Chunk &chunk = chunks[i];
        unsigned int count = chunk.getVertexCount();

        glm::fvec2 delta(chunk.getXZPosition() - playerChunkPosition);
        glm::fvec2 sideA = glm::normalize(glm::fvec2(playerDirection.x, playerDirection.z));
        glm::fvec2 sideB = glm::normalize(glm::fvec2(delta + 0.5f + 2.0f * sideA));
        float angle = glm::dot(sideA, sideB);

        if (count > 0 && angle > CLIP_ANGLE) {
            glDrawArraysInstancedBaseInstance(GL_POINTS, CHUNK_BASE_VERTEX_OFFSET * i, count, 1, i);
        }
    }
}

void World::reloadCurrentChunk() {
    glm::ivec2 playerChunkPosition = toChunkPosition(playerPosition);

    int av = 1;
    Chunk &chunk =  chunks[linearizeChunkPos(playerChunkPosition)];
    chunk.update();
    // chunk.chunkBufferUpdate(av);
}

void World::initializeVertexArray() {
    if (vaoID != 0 || vboID != 0)
        return;

    glCreateVertexArrays(1, &vaoID);
    glCreateBuffers(1, &vboID);
    glCreateBuffers(1, &oboID);

    std::cout << "Vertex Buffer: " << vboID << std::endl;
    std::cout << "Offset Buffer: " << oboID << std::endl;
    int size;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);
    std::cout << "Max Buffer Size: " << size << std::endl;


    chunkBufferSize = std::max((uint64_t)size, maxWorldExtend * maxWorldExtend * CHUNK_BASE_VERTEX_OFFSET * sizeof(st_face));
    glNamedBufferStorage(vboID, chunkBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(oboID, static_cast<GLsizeiptr>(sizeof(glm::fvec3) * maxWorldExtend * maxWorldExtend),
                      nullptr, GL_DYNAMIC_STORAGE_BIT);

    glVertexArrayVertexBuffer(vaoID, 0, vboID, offsetof(st_face, position), sizeof(st_face));
    glVertexArrayAttribIFormat(vaoID, 0, 4, GL_UNSIGNED_BYTE, 0);

    glVertexArrayVertexBuffer(vaoID, 1, vboID, offsetof(st_face, light), sizeof(st_face));
    glVertexArrayAttribIFormat(vaoID, 1, 4, GL_UNSIGNED_BYTE, 0);

    glVertexArrayVertexBuffer(vaoID, 2, vboID, offsetof(st_face, ID), sizeof(st_face));
    glVertexArrayAttribIFormat(vaoID, 2, 1, GL_UNSIGNED_SHORT, 0);

    glVertexArrayVertexBuffer(vaoID, 3, oboID, 0, sizeof(glm::fvec3));
    glVertexArrayAttribFormat(vaoID, 3, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayBindingDivisor(vaoID, 3, 1);

    glEnableVertexArrayAttrib(vaoID, 0);
    glEnableVertexArrayAttrib(vaoID, 1);
    glEnableVertexArrayAttrib(vaoID, 2);
    glEnableVertexArrayAttrib(vaoID, 3);

    hasChunkBufferChanges = false;
}

void World::generateChunk(const glm::ivec2 &position, int threadID) {
    int linIndex = linearizeChunkPos(position);

    if (threadID == -1) {
        chunks[linIndex].generate(position.x, position.y);
        return;
    }
    if (linIndex % threadCount != threadID)
        return;

    Chunk &chunk = chunks[linIndex];
    bool isGenerated = chunk.isGenerated();
    const glm::ivec2 currentPosition = chunk.getXZPosition();
    if (!isGenerated || currentPosition != position) {
        chunk.save();
        chunk.generate(position.x, position.y);
    }
}

void World::updateChunkBuffers() {
    hasChunkBufferChanges = false;
    int availableChanges = CHANGE_CHUNK_MAX;

    for (int i = 0; i < maxWorldExtend * maxWorldExtend; i++) {
        chunks[i].chunkBufferUpdate(availableChanges);
        if (availableChanges <= 0)
            return;
    }
//    std::cout << "[  INFO  ][ World ] Buffer usage: " << std::setprecision(4) << offset * sizeof(st_vertex) / (1024.0 * 1024.0) << "MB\n";
}

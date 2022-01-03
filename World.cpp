/*
 * World.cpp
 *
 *  Created on: Dec 26, 2021
 *      Author: kexx
 */

#include "World.hpp"
#include <iostream>


static const float CLIP_ANGLE = glm::cos(glm::radians(50.0f));
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

    glUnmapNamedBuffer(iboID);
    glDeleteVertexArrays(1, &vaoID);
    glDeleteBuffers(1, &vboID);
    glDeleteBuffers(1, &oboID);
    glDeleteBuffers(1, &iboID);
}

const st_block &World::getBlock(float x, float y, float z) const {
    glm::ivec2 chunkPos, inner;
    toChunkPositionAndOffset(x, z, chunkPos, inner);
    return chunks[linearizeChunkPos(chunkPos)].getBlock(inner.x, (int) y, inner.y);
}

short World::setBlock(float x, float y, float z, short ID, bool forceChunkUpdate) {
    glm::ivec2 chunkPos, inner;
    toChunkPositionAndOffset(x, z, chunkPos, inner);
    return chunks[linearizeChunkPos(chunkPos)].setBlock(inner.x, (int) y, inner.y, ID, forceChunkUpdate);
}


void World::update(int threadID) {
    glm::ivec2 playerChunkPosition = toChunkPosition(playerPosition);

    generateChunk(playerChunkPosition, threadID);
    for (int r = 1; r <= renderDistance; r++) {
        for (const glm::ivec2& offset : tOffset)
            generateChunk(playerChunkPosition + r * offset, threadID);

        for (int s = 1; s < r; s++) {
            for (const glm::ivec2& offset : tJunctionOffset) {
                generateChunk({playerChunkPosition.x + r * offset.x,
                             playerChunkPosition.y + s * offset.y}, threadID);
                generateChunk({playerChunkPosition.x + s * offset.x,
                             playerChunkPosition.y + r * offset.y}, threadID);
            }
        }
        for (const glm::ivec2& offset : tJunctionOffset)
            generateChunk(playerChunkPosition + r * offset, threadID);
    }

    for (int i = threadID; i < maxWorldExtend * maxWorldExtend; i += threadCount) {
        Chunk &chunk = chunks[i];
        glm::fvec2 sideA = glm::normalize(glm::fvec2(playerDirection.x, playerDirection.z));
        glm::fvec2 sideB = glm::normalize(
                glm::fvec2(chunk.getXZPosition() - playerChunkPosition) + 0.5f + 2.0f * sideA);
        float angle = glm::dot(sideA, sideB);
        if (angle > CLIP_ANGLE && chunk.needUpdate())
            chunk.update();
        if (chunk.needBufferUpdate())
            hasChunkBufferChanges = true;
    }
}

void World::updateChunkBuffers() {
    hasChunkBufferChanges = false;
    int availableChanges = CHANGE_CHUNK_MAX;

    for (int i = 0; i < maxWorldExtend * maxWorldExtend; i++) {
        chunks[i].chunkBufferUpdate(availableChanges);
    }

    //std::cout << size * sizeof(st_vertex) / 1024 / 1024 << "MB -- " << 100.0f * (float)size / (float)(maxWorldExtend * maxWorldExtend * CHUNK_BASE_VERTEX_OFFSET) << " %";
    //std::cout << " (" << 100.0f * minUsage / CHUNK_BASE_VERTEX_OFFSET << " % <-> " << 100.0f * maxUsage / CHUNK_BASE_VERTEX_OFFSET << " %)" << std::endl;
}

int World::updateIndirect() {
    if (indirectSync) {
        glDeleteSync(indirectSync);
    }
    indirectSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    int indirectIndex = 0;

    glm::ivec2 playerChunkPosition = toChunkPosition(playerPosition);
    for (int i = 0; i < maxWorldExtend * maxWorldExtend; i++) {
        Chunk &chunk = chunks[i];
        unsigned int count = chunk.getVertexCount();

        glm::fvec2 delta(chunk.getXZPosition() - playerChunkPosition);
        glm::fvec2 sideA = glm::normalize(glm::fvec2(playerDirection.x, playerDirection.z));
        glm::fvec2 sideB = glm::normalize(glm::fvec2(delta + 0.5f + 2.0f * sideA));
        float angle = glm::dot(sideA, sideB);

        if (count > 0 && angle > CLIP_ANGLE) {
            indirectMapping[indirectIndex++] = {
                    count, // vertex count
                    1, // instance count
                    CHUNK_BASE_VERTEX_OFFSET * i, // firstPrimitive Index (not byte offset)
                    static_cast<unsigned>(i) // baseInstance per "glDrawArraysBaseInstance" in multi draw
            };
        }
    }
    if (indirectSync) {
        while (true) {
            GLenum waitReturn = glClientWaitSync(indirectSync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
            if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED)
                break;
        }
    }
    return indirectIndex;
}

void World::render(bool indirect) {
    if (hasChunkBufferChanges)
        updateChunkBuffers();

    glBindVertexArray(vaoID);
    if (indirect) {
        int indirectCount = updateIndirect();
        glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr, indirectCount, 0);
    }
    else {
        glm::ivec2 playerChunkPosition = toChunkPosition(playerPosition);
        for (int i = 0; i < maxWorldExtend * maxWorldExtend; i++) {
            Chunk &chunk = chunks[i];
            unsigned int count = chunk.getVertexCount();

            glm::fvec2 delta(chunk.getXZPosition() - playerChunkPosition);
            glm::fvec2 sideA = glm::normalize(glm::fvec2(playerDirection.x, playerDirection.z));
            glm::fvec2 sideB = glm::normalize(glm::fvec2(delta + 0.5f + 2.0f * sideA));
            float angle = glm::dot(sideA, sideB);

            if (count > 0 && angle > CLIP_ANGLE) {
                glDrawArraysInstancedBaseInstance(GL_TRIANGLES, CHUNK_BASE_VERTEX_OFFSET * i, count, 1, i);
            }
        }
    }
}

void World::reloadCurrentChunk() {
    glm::ivec2 playerChunkPosition = toChunkPosition(playerPosition);

    int av = 1;
    Chunk &chunk =  chunks[linearizeChunkPos(playerChunkPosition)];
    chunk.update();
    chunk.chunkBufferUpdate(av);
}

void World::initializeVertexArray() {
    if (vaoID != 0 || vboID != 0)
        return;

    glCreateVertexArrays(1, &vaoID);
    glCreateBuffers(1, &vboID);
    glCreateBuffers(1, &oboID);
    glCreateBuffers(1, &iboID);

    std::cout << "Vertex   Buffer: " << vboID << std::endl;
    std::cout << "Offset   Buffer: " << oboID << std::endl;
    std::cout << "Indirect Buffer: " << iboID << std::endl;

    glNamedBufferStorage(vboID, static_cast<GLsizei>(maxWorldExtend * maxWorldExtend * CHUNK_BASE_VERTEX_OFFSET * sizeof(st_vertex)),
                      nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(oboID, static_cast<GLsizei>(maxWorldExtend * maxWorldExtend * sizeof(glm::fvec3)),
                      nullptr, GL_DYNAMIC_STORAGE_BIT);

    int indirectSize = static_cast<GLsizei>(maxWorldExtend * maxWorldExtend * sizeof(st_DAIC));
    glNamedBufferStorage(iboID, indirectSize, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    indirectMapping = reinterpret_cast<st_DAIC*>(glMapNamedBufferRange(
            iboID, 0, indirectSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));

    glVertexArrayVertexBuffer(vaoID, 0, vboID, 0, sizeof(st_vertex));
    glVertexArrayAttribFormat(vaoID, 0, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0);

    glVertexArrayVertexBuffer(vaoID, 1, vboID, 4, sizeof(st_vertex));
    glVertexArrayAttribFormat(vaoID, 1, 1, GL_SHORT, GL_FALSE, 0);

    glVertexArrayVertexBuffer(vaoID, 2, oboID, 0, sizeof(glm::fvec3));
    glVertexArrayAttribFormat(vaoID, 2, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayBindingDivisor(vaoID, 2, 1);

    glEnableVertexArrayAttrib(vaoID, 0);
    glEnableVertexArrayAttrib(vaoID, 1);
    glEnableVertexArrayAttrib(vaoID, 2);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, iboID);

    hasChunkBufferChanges = false;
}


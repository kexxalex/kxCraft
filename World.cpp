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
      maxWorldExtend(2 * renderDistance + 3)
{

}

void World::cleanUp() {
    delete[] chunks;
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
    glm::ivec2 playerChunkPosition;
    toChunkPosition(playerPosition, playerChunkPosition);

    if (threadID == 0)
        updateChunk(playerChunkPosition);

    for (int r = 1 + threadID; r <= renderDistance; r += threadCount) {
        for (const glm::ivec2& offset : tOffset) {
            updateChunk(playerChunkPosition + r * offset);
        }

        for (int s = 1; s <= r; s++) {
            for (const glm::ivec2& offset : tJunctionOffset) {
                updateChunk({playerChunkPosition.x + r * offset.x,
                             playerChunkPosition.y + s * offset.y});
                if (s < r) {
                    updateChunk({playerChunkPosition.x + s * offset.x,
                                 playerChunkPosition.y + r * offset.y});
                }
            }

        }
    }
}

void World::updateChunk(const glm::ivec2 &position) {
    Chunk &chunk = chunks[linearizeChunkPos(position)];
    glm::ivec2 cPos = chunk.getXZPosition();
    if (chunk.available || cPos != position) {
        chunkLock.lock();
        chunk.build(&worldGenerator, position.x, position.y);

        const glm::ivec2 north = position + glm::ivec2(0, 1);
        const glm::ivec2 east = position + glm::ivec2(1, 0);
        const glm::ivec2 south = position + glm::ivec2(0, -1);
        const glm::ivec2 west = position + glm::ivec2(-1, 0);

        chunk.setNorth(&chunks[linearizeChunkPos(north)]);
        chunk.setEast(&chunks[linearizeChunkPos(east)]);
        chunk.setSouth(&chunks[linearizeChunkPos(south)]);
        chunk.setWest(&chunks[linearizeChunkPos(west)]);
        chunkLock.unlock();

        chunk.generate();
    }

    if (chunk.needUpdate()) {
        chunk.update();
    }

    if (chunk.needBufferUpdate())
        hasChunkBufferChanges = true;
}

void World::updateChunkBuffers() {
    hasChunkBufferChanges = false;
    int availableChanges = CHANGE_CHUNK_MAX;
    for (int i = 0; i < maxWorldExtend * maxWorldExtend; i++) {
        Chunk &chunk = chunks[i];
        chunk.chunkBufferUpdate(availableChanges, oboID);
    }
}

void World::updateIndirect() {
    glm::ivec2 playerChunkPosition;
    toChunkPosition(playerPosition, playerChunkPosition);

    int indirectIndex = 0;
    auto indirect = (st_DAIC*)glMapNamedBuffer(iboID, GL_WRITE_ONLY);

    for (int i = 0; i < maxWorldExtend * maxWorldExtend; i++) {
        Chunk &chunk = chunks[i];
        unsigned int count = chunk.getVertexCount();

        glm::fvec2 sideA = glm::normalize(glm::fvec2(playerDirection.x, playerDirection.z));
        glm::fvec2 sideB = glm::normalize(
                glm::fvec2(chunk.getXZPosition() - playerChunkPosition) + 0.5f + 2.0f * sideA);
        float angle = glm::dot(sideA, sideB);

        if (count > 0 && angle > CLIP_ANGLE) {
            indirect[indirectIndex++] = {count, 1, CHUNK_BASE_VERTEX_OFFSET * i, static_cast<unsigned int>(i)};
        }
    }

    indirectCount = indirectIndex;
    glUnmapNamedBuffer(iboID);
}

void World::render(Shader &shader) {
    if (hasChunkBufferChanges)
        updateChunkBuffers();
    updateIndirect();

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, iboID);
    glBindVertexArray(vaoID);
    glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr, indirectCount, 0);
}

void World::reloadCurrentChunk() {
    glm::ivec2 playerChunkPosition;
    toChunkPosition(playerPosition, playerChunkPosition);

    int av = 1;
    Chunk &chunk =  chunks[linearizeChunkPos(playerChunkPosition)];
    chunk.update();
    chunk.chunkBufferUpdate(av, oboID);
}

void World::initializeVertexArray() {
    if (vaoID != 0 || vboID != 0)
        return;

    glCreateVertexArrays(1, &vaoID);
    glCreateBuffers(1, &vboID);
    glCreateBuffers(1, &oboID);
    glCreateBuffers(1, &iboID);

    glNamedBufferData(vboID, static_cast<GLsizei>(maxWorldExtend * maxWorldExtend * CHUNK_BASE_VERTEX_OFFSET * sizeof(st_vertex)),
                      nullptr, GL_STATIC_DRAW);
    glNamedBufferData(oboID, static_cast<GLsizei>(maxWorldExtend * maxWorldExtend * sizeof(glm::fvec3)),
                      nullptr, GL_STATIC_DRAW);
    glNamedBufferData(iboID, static_cast<GLsizei>(maxWorldExtend * maxWorldExtend * sizeof(st_DAIC)),
                      nullptr, GL_STREAM_DRAW);

    for (int i = 0; i < maxWorldExtend * maxWorldExtend; i++) {
        chunks[i].setBufferOffset(vboID, i);
    }

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

    hasChunkBufferChanges = true;
}

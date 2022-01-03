/*
 * World.h
 *
 *  Created on: Dec 26, 2021
 *      Author: kexx
 */

#pragma once

#include <vector>
#include "Chunk.hpp"
#include <unordered_map>
#include <iostream>
#include <fstream>


static constexpr int CHANGE_CHUNK_MAX = 1024;

inline int MOD(int n, int m) {
    return n % m + m * (n < 0);
}

struct st_DAIC {
    unsigned int count;
    unsigned int instanceCount;
    unsigned int first;
    unsigned int baseInstance;
};


class World {
public:
    World() = delete;
    World(const glm::fvec3 &start_position, int seed, int renderDistance = 4, int threadCount = 1);
    ~World();

    void update(int threadID);

    inline void setPlayer(const glm::fvec3 &position, const glm::fvec3 &direction) {
        playerPosition = position;
        playerDirection = direction;
    }

    void render(bool indirect);

    void setInactive() { m_active = false; }

    short setBlock(float x, float y, float z, short ID, bool chunkUpdate=false);
    [[nodiscard]] const st_block &getBlock(float x, float y, float z) const;

    void reloadCurrentChunk();
    void initializeVertexArray();

    [[nodiscard]] inline float getRenderDistance() const { return static_cast<float>(renderDistance * C_EXTEND); }
    [[nodiscard]] inline bool isActive() const { return m_active; }
    [[nodiscard]] inline int linearizeChunkPos(const glm::ivec2 &chunkPosition) const {
        return MOD(
                MOD(chunkPosition.y, maxWorldExtend) * maxWorldExtend + MOD(chunkPosition.x, maxWorldExtend),
                maxWorldExtend*maxWorldExtend);
    }

private:
    void updateChunkBuffers();
    int updateIndirect();

    inline void generateChunk(const glm::ivec2 &position, int threadID) {
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

    static inline glm::ivec2 toChunkPosition(float x, float z) {
        return { glm::floor(x / C_EXTEND), glm::floor(z / C_EXTEND) };
    }
    static inline glm::ivec2 toChunkPosition(const glm::fvec3 &position) {
        return toChunkPosition(position.x, position.z);
    }

    static inline void toChunkPositionAndOffset(float x, float z, glm::ivec2 &chunkPos, glm::ivec2 &inner) {
        chunkPos = toChunkPosition(x, z);
        inner = glm::ivec2(glm::floor(x - (float)chunkPos.x * C_EXTEND), glm::floor(z - (float)chunkPos.y * C_EXTEND));
    }

    bool m_active{ true };
    int renderDistance{ 6 };
    int maxWorldExtend{ 15 };
    int threadCount{ 1 };

    WorldGenerator worldGenerator;

    glm::fvec3 playerPosition{0, 0, 0};
    glm::fvec3 playerDirection{0, 0, 1};

    unsigned int vaoID{ 0 };
    unsigned int vboID{ 0 };
    unsigned int oboID{ 0 };
    unsigned int iboID{ 0 };

    bool hasChunkBufferChanges{ false };

    GLsync indirectSync;
    st_DAIC* indirectMapping{ nullptr };
    Chunk* chunks{ nullptr };
};

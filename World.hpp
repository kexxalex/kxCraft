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
#include <mutex>
#include <iostream>


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
    World() = default;
    World(const glm::fvec3 &start_position, int seed, unsigned int renderDistance = 4, unsigned int threadCount = 2);

    World &operator=(const World &world)
    {
        renderDistance = world.renderDistance;
        threadCount = world.threadCount;
        worldGenerator = world.worldGenerator;
        playerPosition = world.playerPosition;
        maxWorldExtend = 2 * renderDistance + 3;
        chunks = new Chunk[maxWorldExtend * maxWorldExtend];

        glm::ivec2 chunkPos;

        toChunkPosition(playerPosition, chunkPos);
        chunks[linearizeChunkPos(chunkPos)].build(&worldGenerator, chunkPos.x, chunkPos.y);
        chunks[linearizeChunkPos(chunkPos)].generate();

        return *this;
    }

    void update(int threadID);
    void cleanUp();

    inline void setPlayer(const glm::fvec3 &position, const glm::fvec3 &direction) {
        playerPosition = position;
        playerDirection = direction;
    }

    void render(Shader &shader);

    void setInactive() { m_active = false; }

    short setBlock(float x, float y, float z, short ID, bool chunkUpdate=false);
    [[nodiscard]] const st_block &getBlock(float x, float y, float z) const;

    void reloadCurrentChunk();
    void initializeVertexArray();

    [[nodiscard]] inline float getRenderDistance() const { return static_cast<float>(renderDistance * C_EXTEND); }
    [[nodiscard]] inline bool isActive() const { return m_active; }
    [[nodiscard]] inline int linearizeChunkPos(const glm::ivec2 &chunkPos) const {
        return MOD(chunkPos.y + renderDistance + 1, maxWorldExtend) * maxWorldExtend + MOD(chunkPos.x + renderDistance + 1, maxWorldExtend);
    }

private:
    void updateChunk(const glm::ivec2 &position, int threadID);
    void updateChunkBuffers();
    void updateIndirect();

    static inline void toChunkPosition(float x, float z, glm::ivec2 &chunkPos) {
        chunkPos = glm::ivec2( glm::floor(x / C_EXTEND), glm::floor(z / C_EXTEND) );
    }
    static inline void toChunkPosition(const glm::fvec3 &position, glm::ivec2 &chunkPos) {
        return toChunkPosition(position.x, position.z, chunkPos);
    }

    static inline void toChunkPositionAndOffset(float x, float z, glm::ivec2 &chunkPos, glm::ivec2 &inner) {
        chunkPos = glm::ivec2( glm::floor(x / C_EXTEND), glm::floor(z / C_EXTEND) );
        inner = glm::ivec2(glm::floor(x - (float)chunkPos.x * C_EXTEND), glm::floor(z - (float)chunkPos.y * C_EXTEND));
    }
    static inline void toChunkPositionAndOffset(const glm::fvec3 &position, glm::ivec2 &chunkPos, glm::ivec2 &inner) {
        toChunkPositionAndOffset(position.x, position.z, chunkPos, inner);
    }

    bool m_active{ true };
    unsigned int renderDistance{ 6 };
    unsigned int threadCount{ 1 };

    WorldGenerator worldGenerator;

    glm::fvec3 playerPosition{0, 0, 0};
    glm::fvec3 playerDirection{0, 0, 1};

    unsigned int vaoID{ 0 };
    unsigned int vboID{ 0 };
    unsigned int oboID{ 0 };
    unsigned int iboID{ 0 };

    unsigned int indirectCount{ 0 };
    bool hasChunkBufferChanges{ false };

    unsigned int maxWorldExtend{ 0 };
    Chunk* chunks{ nullptr };
};

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


static constexpr float FOV = 65.0f;
static constexpr int CHANGE_CHUNK_MAX = 4096;


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

    void updateChunkBuffers();
    void render() const;

    void setInactive() { m_active = false; }

    [[nodiscard]] constexpr bool needBufferUpdate() const { return hasChunkBufferChanges; }

    short setBlock(float x, float y, float z, short ID, bool chunkUpdate=false);
    [[nodiscard]] inline const st_block &getBlock(float x, float y, float z) const {
        glm::ivec2 chunkPos, inner;
        toChunkPositionAndOffset(x, z, chunkPos, inner);
        return chunks[linearizeChunkPos(chunkPos)].getBlock(inner.x, (int) y, inner.y);
    }
    [[nodiscard]] const Block &getBlockAttributes(float x, float y, float z) const {
        glm::ivec2 chunkPos, inner;
        toChunkPositionAndOffset(x, z, chunkPos, inner);
        return chunks[linearizeChunkPos(chunkPos)].getBlockAttributes(inner.x, (int) y, inner.y);
    }

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

    void generateChunk(const glm::ivec2 &position, int threadID);
    void updateChunk(const glm::ivec2 &position, const glm::ivec2 &playerChunkPosition, int threadID);

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
    GLsizeiptr chunkBufferSize{ 0 };
    unsigned int totalVertices{ 0 };

    bool hasChunkBufferChanges{ false };

    Chunk* chunks{ nullptr };
};

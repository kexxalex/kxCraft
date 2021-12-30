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


constexpr int CHANGE_CHUNK_MAX = 1;


struct hash_iVec2 {
    size_t operator()(const glm::ivec2 &p) const {
        return std::hash<int>{}(p.x) ^ std::hash<int>{}(p.y);
    }
};

class World {
public:
    World() = default;
    World(const glm::fvec3 &start_position, int seed, int renderDistance = 4, int threadCount = 2);

    World& operator=(const World& world) {
        renderDistance = world.renderDistance;
        threadCount = world.threadCount;
        worldGenerator = world.worldGenerator;
        playerDirection = world.playerDirection;
        playerPosition = world.playerPosition;

        chunks.reserve((2*renderDistance+1) * (2*renderDistance+1));

        glm::ivec2 chunkPos = glm::ivec2(
                glm::floor(playerPosition.x / C_EXTEND),
                glm::floor(playerPosition.z / C_EXTEND)
        ) * C_EXTEND;

        chunkLock.lock();
        chunks[chunkPos] = Chunk(&worldGenerator, chunkPos.x, chunkPos.y);
        chunks[chunkPos].generate();
        chunkLock.unlock();

        return *this;
    }

    ~World();

    void update(int threadID);
    void cleanUp();

    inline void setPlayer(const glm::fvec3 &position, const glm::fvec3 &direction) {
        playerPosition = position;
        playerDirection = direction;
    }

    void render();

    void setInactive() { m_active = false; }

    short setBlock(float x, float y, float z, short ID, bool chunkUpdate=false);

    const st_block &getBlock(float x, float y, float z) const;;

    void reloadCurrentChunk();

    [[nodiscard]] inline float getRenderDistance() const { return static_cast<float>(renderDistance * C_EXTEND); }
    [[nodiscard]] inline bool isActive() const { return m_active; }

private:
    void updateChunk(const glm::ivec2 &position);

    bool m_active{true};
    int renderDistance{6};
    int threadCount{1};

    glm::fvec3 playerPosition{0, 0, 0};
    glm::fvec3 playerDirection{0, 0, 1};

    WorldGenerator worldGenerator;
    std::mutex chunkLock;
    std::unordered_map<const glm::ivec2, Chunk, hash_iVec2> chunks;
};

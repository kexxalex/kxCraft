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


constexpr int CHANGE_CHUNK_MAX = 4;


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
        chunks.reserve((2*renderDistance+1) * (2*renderDistance+1));

        return *this;
    }

    ~World();

    void update(int threadID);

    inline void setPlayer(const glm::fvec3 &position, const glm::fvec3 &direction) {
        playerPosition = position;
        playerDirection = direction;
    }

    void render();

    void setInactive() { m_active = false; }

    const st_block &getBlock(float x, float y, float z) const {
        const glm::ivec2 chunkPos = glm::ivec2(
                glm::floor(x / C_EXTEND),
                glm::floor(z / C_EXTEND)
        ) * C_EXTEND;
        glm::ivec2 inner = glm::ivec2(x, z) - chunkPos;
        return (y < 0 || y >= C_HEIGHT || chunks.find(chunkPos) == chunks.end()) ?
               AIR_BLOCK : chunks.at(chunkPos).getBlock(inner.x, (int) y, inner.y);
    };

    inline void reloadCurrentChunk() {
        const glm::ivec2 chunkPos = glm::ivec2(
                glm::floor(playerPosition.x / C_EXTEND),
                glm::floor(playerPosition.z / C_EXTEND)
        ) * C_EXTEND;
        if (chunks.find(chunkPos) != chunks.end())
            chunks[chunkPos].update();
    }

    [[nodiscard]] inline float getRenderDistance() const { return static_cast<float>(renderDistance * C_EXTEND); }
    [[nodiscard]] inline bool isActive() const { return m_active; }

private:
    void addChunk(const glm::ivec2 &position);

    bool m_active{true};
    int renderDistance{6};
    int threadCount{1};

    glm::fvec3 playerPosition{0, 0, 0};
    glm::fvec3 playerDirection{0, 0, 1};

    WorldGenerator worldGenerator;
    std::mutex chunkLock;
    std::unordered_map<const glm::ivec2, Chunk, hash_iVec2> chunks;
};

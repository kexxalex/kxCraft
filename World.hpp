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
    World(const glm::fvec3 &start_position, int seed, int renderDistance = 4, int threadCount = 2);

    ~World();

    void update(int threadID, glm::fvec3 *position, glm::fvec3 *direction);
    void render(const glm::fvec3 &position, const glm::fvec3 &direction);
    void setInactive() { m_active = false; }

    const st_block& getBlock(float x, float y, float z) const {
        const glm::ivec2 chunkPos = glm::ivec2(
                glm::floor(x / C_EXTEND),
                glm::floor(z / C_EXTEND)
        ) * C_EXTEND;
        glm::ivec2 inner = glm::ivec2(x, z) - chunkPos;
        return (y < 0 || y >= C_HEIGHT || chunks.find(chunkPos) == chunks.end()) ?
            AIR_BLOCK : chunks.at(chunkPos).getBlock(inner.x, (int)y, inner.y);
    };

    [[nodiscard]] inline bool isActive() const { return m_active; }

private:
    void addChunk(const glm::ivec2 &chunkPos, const glm::ivec2 &position, const glm::fvec3 &direction);
    bool m_active{true};
    int renderDistance{ 6 };
    int threadCount{ 1 };
    WorldGenerator worldGenerator;
    std::mutex chunkLock;
    std::unordered_map<const glm::ivec2, Chunk, hash_iVec2> chunks;
};

/*
 * Chunk.hpp
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <mutex>

#include "Item.hpp"
#include "Block.hpp"
#include "WorldGenerator.hpp"

struct st_vertex {
    glm::fvec3 position{ 0, 0, 0};
    short ID{ 0 };
    short light{ 0 };

    st_vertex() = default;
    st_vertex(const glm::fvec3& pos, short texID, short light)
        : position(pos), ID(texID), light(light)
    {

    }
};

class Chunk {
public:
    Chunk() = default;
    Chunk(WorldGenerator *world_generator, int x, int z);
    ~Chunk();

    Chunk &operator=(const Chunk &c) {
        m_worldGenerator = c.m_worldGenerator;
        m_position = c.m_position;
        return *this;
    }

    void update();
    void generate();
    void render();

    inline Chunk* const getNorth() const { return m_north; }
    inline Chunk* const getEast() const { return m_east; }
    inline Chunk* const getSouth() const { return m_south; }
    inline Chunk* const getWest() const { return m_west; }

    inline void setNorth(Chunk* chunk_ptr, bool itsNeighbour=false) {
        m_needUpdate = true;
        m_north = chunk_ptr;
        if (!itsNeighbour && m_north != nullptr)
            m_north->setSouth(this, true);
    }
    inline void setEast(Chunk* chunk_ptr, bool itsNeighbour=false) {
        m_needUpdate = true;
        m_east = chunk_ptr;
        if (!itsNeighbour && m_east != nullptr)
            m_east->setWest(this, true);
    }
    inline void const setSouth(Chunk* chunk_ptr, bool itsNeighbour=false) {
        m_needUpdate = true;
        m_south = chunk_ptr;
        if (!itsNeighbour && m_south != nullptr)
            m_south->setNorth(this, true);
    }
    inline void const setWest(Chunk* chunk_ptr, bool itsNeighbour=false) {
        m_needUpdate = true;
        m_west = chunk_ptr;
        if (!itsNeighbour && m_west != nullptr)
            m_west->setEast(this, true);
    }

    [[nodiscard]] inline bool needUpdate() const { return m_needUpdate; }
    [[nodiscard]] inline bool isGenerated() const { return m_generated; }
    [[nodiscard]] inline bool isTransparent(int x, int y, int z) const {
        return BLOCK_PROP[getBlock(x, y, z).block].transparent;
    }

    [[nodiscard]] glm::ivec2 getPosition() const { return {m_position.x, m_position.z}; }
    [[nodiscard]] const st_block& getBlock(int x, int y, int z) const;

    inline void setBlock(int x, int y, int z, unsigned char block = AIR) {
        if (block != m_blocks[linearizeCoord(x, y, z)].block)
            m_needUpdate = true;
        m_blocks[linearizeCoord(x, y, z)].block = block;
    }

    [[nodiscard]] inline short getCornerLight(int x, int y, int z);


private:
    bool m_needUpdate{ true };
    Chunk *m_north{ nullptr }, *m_east{ nullptr }, *m_south{ nullptr }, *m_west{ nullptr };

    [[nodiscard]] st_block& getBlockRef(int x, int y, int z);
    bool m_generated{ false };
    void initializeVertexArray();

    void addFace(short ID, const glm::fvec3 &pos, const glm::fvec3 &edgeA, const glm::fvec3 &edgeB);
    void addCube(int x, int y, int z, short block);
    void fillSunlight();
    void updateBlockLight(int x, int y, int z, std::vector<glm::ivec3> &updateBlocks);

    glm::fvec3 m_position{0, 0, 0};
    WorldGenerator *m_worldGenerator{ nullptr };
    bool m_hasVertexUpdate{ false };

    st_block m_blocks[C_EXTEND * C_EXTEND * C_HEIGHT];

    int vertexCount{0};
    int currentBufferSize{0};
    unsigned int vaoID{0};
    unsigned int vboID{0};

    std::vector<st_vertex> m_vertices;
};


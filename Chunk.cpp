/*
 * Chunk.cpp
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#include "Chunk.hpp"
#include "Block.hpp"
#include <GL/glew.h>
#include <iostream>



Chunk::Chunk(WorldGenerator *worldGenerator, int x, int z)
        : m_position(x, 0, z), m_worldGenerator(worldGenerator),
          m_north{nullptr}, m_east{nullptr}, m_south{nullptr}, m_west{nullptr} {
}

Chunk &Chunk::operator=(const Chunk &c) {
    m_worldGenerator = c.m_worldGenerator;
    m_position = c.m_position;
    return *this;
}

Chunk::~Chunk() {
    chunkDestructionLock.lock();
    if (!m_generated)
        return;
    // std::cout << "Delete Chunk " << m_position.x << ", " << m_position.z << " -- " << vaoID << std::endl;
    m_hasVertexUpdate = false;
    vertexCount = 0;
    m_vertices.clear();
    if (vboID != 0)
        glDeleteBuffers(1, &vboID);
    if (vaoID != 0)
        glDeleteVertexArrays(1, &vaoID);

    if (nullptr != m_north) {
        m_north->m_south = nullptr;
    }
    if (nullptr != m_east) {
        m_east->m_west = nullptr;
    }
    if (nullptr != m_south) {
        m_south->m_north = nullptr;
    }
    if (nullptr != m_west) {
        m_west->m_east = nullptr;
    }
}

void Chunk::generate() {
    for (int z = 0; z < C_EXTEND; z++) {
        for (int x = 0; x < C_EXTEND; x++) {
            m_worldGenerator->placeStack(
                    x + (int) m_position.x, z + (int) m_position.z,
                    &m_blocks[linearizeCoord(x, 0, z)]);
        }
    }
    fillSunlight();
    m_generated = true;
}

st_block& Chunk::getBlockRef(int x, int y, int z) {
    bool outOfBounds = (x < 0 || y < 0 || z < 0 || x >= C_EXTEND || z >= C_EXTEND || y >= C_HEIGHT);
    if (!outOfBounds)
        return m_blocks[linearizeCoord(x, y, z)];
    else if (z >= C_EXTEND && m_north != nullptr)
        return m_north->getBlockRef(x, y, z - C_EXTEND);
    else if (x >= C_EXTEND && m_east != nullptr)
        return m_east->getBlockRef(x - C_EXTEND, y, z);
    else if (z < 0 && m_south != nullptr)
        return m_south->getBlockRef(x, y, z + C_EXTEND);
    else if (x < 0 && m_west != nullptr)
        return m_west->getBlockRef(x + C_EXTEND, y, z);
    return AIR_BLOCK;
}

const st_block& Chunk::getBlock(int x, int y, int z) const {
    bool outOfBounds = (x < 0 || y < 0 || z < 0 || x >= C_EXTEND || z >= C_EXTEND || y >= C_HEIGHT);
    if (!outOfBounds)
        return m_blocks[linearizeCoord(x, y, z)];
    else if (z >= C_EXTEND && m_north != nullptr)
        return m_north->getBlock(x, y, z - C_EXTEND);
    else if (x >= C_EXTEND && m_east != nullptr)
        return m_east->getBlock(x - C_EXTEND, y, z);
    else if (z < 0 && m_south != nullptr)
        return m_south->getBlock(x, y, z + C_EXTEND);
    else if (x < 0 && m_west != nullptr)
        return m_west->getBlock(x + C_EXTEND, y, z);
    return AIR_BLOCK;
}

short Chunk::getCornerLight(int x, int y, int z) {
    //neighbourLock.lock();
    const st_block& b111 = getBlock(x, y, z);
    const st_block& b101 = getBlock(x-1, y, z);
    const st_block& b110 = getBlock(x, y, z-1);
    const st_block& b100 = getBlock(x-1, y, z-1);

    const st_block& b011 = getBlock(x, y - 1, z);
    const st_block& b001 = getBlock(x-1, y - 1, z);
    const st_block& b010 = getBlock(x, y - 1, z-1);
    const st_block& b000 = getBlock(x-1, y - 1, z-1);
    //neighbourLock.unlock();

    auto totalLight = (
            (b111.getLight() + b101.getLight() + b110.getLight() + b100.getLight() +
            b011.getLight() + b001.getLight() + b010.getLight() + b000.getLight())
    );

    return static_cast<short>(totalLight);
}

void Chunk::addFace(short ID, const glm::fvec3 &pos,
                    const glm::fvec3 &edgeA, const glm::fvec3 &edgeB) {

    glm::ivec3 p00(pos);
    glm::ivec3 p10(pos + edgeA);
    glm::ivec3 p01(pos + edgeB);
    glm::ivec3 p11(pos + edgeA + edgeB);
    m_vertices.emplace_back(m_position + pos, ID, getCornerLight(p00.x, p00.y, p00.z));
    m_vertices.emplace_back(m_position + pos + edgeA, getCornerLight(p11.x, p11.y, p11.z), getCornerLight(p10.x, p10.y, p10.z));
    m_vertices.emplace_back(m_position + pos + edgeB, 0, getCornerLight(p01.x, p01.y, p01.z));
}

void Chunk::addCube(int x, int y, int z, short block) {
    // Add top and bottom face
    const bool& connect = BLOCKS[block].connect;
    if (isTransparent(x, y + 1, z))
        addFace(BLOCKS[block].top,
                glm::fvec3(x, y + 1, z),
                glm::fvec3(0, 0, 1),
                glm::fvec3(1, 0, 0));
    if (isTransparent(x, y - 1, z))
        addFace(BLOCKS[block].bottom,
                glm::fvec3(x, y, z),
                glm::fvec3(1, 0, 0),
                glm::fvec3(0, 0, 1));


    // Add north and south face
    if (isTransparent(x, y, z + 1))
        addFace((connect && getBlock(x, y-1, z+1).ID == block) ? BLOCKS[block].top : BLOCKS[block].north,
                glm::fvec3(x, y, z + 1),
                glm::fvec3(1, 0, 0),
                glm::fvec3(0, 1, 0));
    if (isTransparent(x, y, z - 1))
        addFace((connect && getBlock(x, y-1, z-1).ID == block) ? BLOCKS[block].top : BLOCKS[block].south,
                glm::fvec3(x + 1, y, z),
                glm::fvec3(-1, 0, 0),
                glm::fvec3(0, 1, 0));


    // Add east and west face
    if (isTransparent(x - 1, y, z)) {
        addFace((connect && getBlock(x-1, y-1, z).ID == block) ? BLOCKS[block].top : BLOCKS[block].west,
                glm::fvec3(x, y, z),
                glm::fvec3(0, 0, 1),
                glm::fvec3(0, 1, 0));
    }
    if (isTransparent(x + 1, y, z))
        addFace((connect && getBlock(x+1, y-1, z).ID == block) ? BLOCKS[block].top : BLOCKS[block].east,
                glm::fvec3(x + 1, y, z + 1),
                glm::fvec3(0, 0, -1),
                glm::fvec3(0, 1, 0));
}

void Chunk::update() {
    if (!chunkDestructionLock.try_lock())
        return;

    std::vector<glm::ivec3> updateBlocks;
    for (int z = 0; z < C_EXTEND; z++) {
        for (int x = 0; x < C_EXTEND; x++) {
            for (int y = C_HEIGHT - 1; y >= 0; y--) {
                updateBlockLight(x, y, z, updateBlocks);
            }
        }
    }

    for (int i=0; i < updateBlocks.size(); i++) {
        glm::ivec3 &p = updateBlocks[i];
        updateBlockLight(p.x, p.y, p.z, updateBlocks);
    }

    vertexLock.lock();
    m_vertices.clear();

    for (int z = 0; z < C_EXTEND; z++) {
        for (int x = 0; x < C_EXTEND; x++) {
            for (int y = 0; y < C_HEIGHT; y++) {
                const st_block& block = m_blocks[linearizeCoord(x, y, z)];
                if (block.ID == BLOCK_ID::AIR)
                    continue;

                switch (BLOCKS[block.ID].type) {
                    case R_BLOCK:
                        addCube(x, y, z, block.ID);
                        break;
                    case R_CROSS:
                        break;
                }
            }
        }
    }

    m_needUpdate = false;
    vertexLock.unlock();
    m_hasVertexUpdate = true;
    chunkDestructionLock.unlock();
}

void Chunk::initializeVertexArray() {
    glCreateVertexArrays(1, &vaoID);
    glCreateBuffers(1, &vboID);

    glBindVertexArray(vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(st_vertex), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_SHORT, GL_FALSE, sizeof(st_vertex), (void *) 12);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Chunk::render() {
    if (!m_generated)
        return;
    if (vaoID == 0 && vboID == 0)
        initializeVertexArray();

    if (m_hasVertexUpdate) {
        m_hasVertexUpdate = false;
        vertexCount = static_cast<int>(m_vertices.size());

        if (vertexCount == 0) {
            return;
        }

        int newBufferSize = static_cast<int>(vertexCount * sizeof(glm::fvec4));
        if (newBufferSize <= currentBufferSize) {
            glNamedBufferSubData(vboID, 0, newBufferSize, &m_vertices[0].position.x);
        } else {
            glNamedBufferData(vboID, newBufferSize, &m_vertices[0].position.x, GL_STATIC_DRAW);
            currentBufferSize = newBufferSize;
        }
    }

    if (vertexCount > 0) {
        glBindVertexArray(vaoID);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
}

void Chunk::fillSunlight() {
    for (int z = 0; z < C_EXTEND; z++) {
        for (int x = 0; x < C_EXTEND; x++) {
            for (int y = C_HEIGHT-1; y >= 0 && m_blocks[linearizeCoord(x, y, z)].ID == AIR; y--) {
                m_blocks[linearizeCoord(x, y, z)].sunLight = MAX_SUN_LIGHT;
            }
        }
    }
}

void Chunk::updateBlockLight(int x, int y, int z, std::vector<glm::ivec3> &updateBlocks) {
    // Discard update, if block is not in this chunk
    if (x < 0 || y < 0 || z < 0 || x >= C_EXTEND || z >= C_EXTEND || y >= C_HEIGHT)
        return;

    st_block &current(getBlockRef(x, y, z));
    st_block &t(getBlockRef(x, y + 1, z));
    st_block &b(getBlockRef(x, y - 1, z));

    st_block &n(getBlockRef(x, y, z + 1));
    st_block &e(getBlockRef(x + 1, y, z));
    st_block &s(getBlockRef(x, y, z - 1));
    st_block &w(getBlockRef(x - 1, y, z));

    int maxSunLight = glm::max(
            glm::max(t.getSunLight(), b.getSunLight()),
            glm::max(
                    glm::max(n.getSunLight(), e.getSunLight()),
                    glm::max(s.getSunLight(), w.getSunLight())
            )
    );

    if (current.ID == BLOCK_ID::AIR && maxSunLight > current.sunLight + 1) {
        current.sunLight = maxSunLight - 1;

        bool inBounds = (x > 0 && y > 0 & z > 0 && x+1 < C_EXTEND && z+1 < C_EXTEND && y+1 < C_HEIGHT);

        if (t.sunLight != NO_LIGHT && glm::abs(t.sunLight - current.sunLight) > 1 && inBounds)
        {
            updateBlocks.emplace_back(x, y+1, z);
        }
        if (b.sunLight != NO_LIGHT && glm::abs(b.sunLight - current.sunLight) > 1 && inBounds) {
            updateBlocks.emplace_back(x, y-1, z);
        }

        if (n.sunLight != NO_LIGHT && glm::abs(n.sunLight - current.sunLight) > 1) {
            if (inBounds)
                updateBlocks.emplace_back(x, y, z+1);
            else {
                if (m_north != nullptr)
                    m_north->m_needUpdate = true;
            }
        }
        if (e.sunLight != NO_LIGHT && glm::abs(e.sunLight - current.sunLight) > 1 && inBounds) {
            if (inBounds)
                updateBlocks.emplace_back(x+1, y, z);
            else {
                if (m_east != nullptr)
                    m_east->m_needUpdate = true;
            }
        }
        if (s.sunLight != NO_LIGHT && glm::abs(s.sunLight - current.sunLight) > 1 && inBounds) {
            if (inBounds)
                updateBlocks.emplace_back(x, y, z-1);
            else {
                if (m_south != nullptr)
                    m_south->m_needUpdate = true;
            }
        }
        if (w.sunLight != NO_LIGHT && glm::abs(w.sunLight - current.sunLight) > 1 && inBounds) {
            if (inBounds)
                updateBlocks.emplace_back(x-1, y, z);
            else {
                if (m_west != nullptr)
                    m_west->m_needUpdate = true;
            }
        }
    }
}

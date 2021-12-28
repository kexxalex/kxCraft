/*
 * Chunk.cpp
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#include "Chunk.hpp"
#include "Block.hpp"
#include <GL/glew.h>



Chunk::Chunk(WorldGenerator *worldGenerator, int x, int z)
        : m_position(x, 0, z), m_worldGenerator(worldGenerator),
          north{nullptr}, east{nullptr}, south{nullptr}, west{nullptr} {
}

Chunk::~Chunk() {
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

    if (nullptr != north)
        north->south = nullptr;
    if (nullptr != east)
        east->west = nullptr;
    if (nullptr != south)
        south->north = nullptr;
    if (nullptr != west)
        west->east = nullptr;
}

void Chunk::generate() {
    m_generated = true;
    for (int z = 0; z < C_EXTEND; z++) {
        for (int x = 0; x < C_EXTEND; x++) {
            m_worldGenerator->placeStack(
                    x + (int) m_position.x, z + (int) m_position.z,
                    &m_blocks[linearizeCoord(x, 0, z)]);
        }
    }
}

[[nodiscard]] inline short Chunk::getCornerLight(int x, int y, int z) const {
    const st_block& b111 = getBlock(x, y, z);
    const st_block& b101 = getBlock(x-1, y, z);
    const st_block& b110 = getBlock(x, y, z-1);
    const st_block& b100 = getBlock(x-1, y, z-1);

    const st_block& b011 = getBlock(x, y - 1, z);
    const st_block& b001 = getBlock(x-1, y - 1, z);
    const st_block& b010 = getBlock(x, y - 1, z-1);
    const st_block& b000 = getBlock(x-1, y - 1, z-1);

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
    if (isTransparent(x - 1, y, z)) {
        addFace((BLOCK_PROP[block].connect && getBlock(x-1, y-1, z).block == block)
                    ? BLOCK_PROP[block].top
                    : BLOCK_PROP[block].west,
                glm::fvec3(x, y, z),
                glm::fvec3(0, 0, 1),
                glm::fvec3(0, 1, 0));
    }
    if (isTransparent(x + 1, y, z))
        addFace((BLOCK_PROP[block].connect && getBlock(x+1, y-1, z).block == block)
                ? BLOCK_PROP[block].top
                : BLOCK_PROP[block].east,
                glm::fvec3(x + 1, y, z + 1),
                glm::fvec3(0, 0, -1),
                glm::fvec3(0, 1, 0));


    if (isTransparent(x, y + 1, z))
        addFace(BLOCK_PROP[block].top,
                glm::fvec3(x, y + 1, z),
                glm::fvec3(0, 0, 1),
                glm::fvec3(1, 0, 0));
    if (isTransparent(x, y - 1, z))
        addFace(BLOCK_PROP[block].bottom,
                glm::fvec3(x, y, z),
                glm::fvec3(1, 0, 0),
                glm::fvec3(0, 0, 1));

    if (isTransparent(x, y, z + 1))
        addFace((BLOCK_PROP[block].connect && getBlock(x, y-1, z+1).block == block)
                    ? BLOCK_PROP[block].top
                    : BLOCK_PROP[block].north,
                glm::fvec3(x, y, z + 1),
                glm::fvec3(1, 0, 0),
                glm::fvec3(0, 1, 0));
    if (isTransparent(x, y, z - 1))
        addFace((BLOCK_PROP[block].connect && getBlock(x, y-1, z-1).block == block)
                    ? BLOCK_PROP[block].top
                    : BLOCK_PROP[block].south,
                glm::fvec3(x + 1, y, z),
                glm::fvec3(-1, 0, 0),
                glm::fvec3(0, 1, 0));
}

void Chunk::update() {
    vertexLock.lock();
    m_needUpdate = false;
    m_vertices.clear();

    for (int z = 0; z < C_EXTEND; z++) {
        for (int x = 0; x < C_EXTEND; x++) {
            for (int y = 0; y < C_HEIGHT; y++) {
                const st_block& block = m_blocks[linearizeCoord(x, y, z)];
                if (block.block == BLOCK_ID::AIR)
                    continue;

                switch (BLOCK_PROP[block.block].type) {
                    case R_BLOCK:
                        addCube(x, y, z, block.block);
                        break;
                    case R_CROSS:
                        break;
                }
            }
        }
    }

    m_hasVertexUpdate = true;
    vertexLock.unlock();
}

void Chunk::initializeVertexArray() {
    glGenVertexArrays(1, &vaoID);
    glGenBuffers(1, &vboID);

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

    if (m_hasVertexUpdate && vertexLock.try_lock()) {
        m_hasVertexUpdate = false;
        vertexCount = static_cast<int>(m_vertices.size());

        if (vertexCount == 0) {
            vertexLock.unlock();
            return;
        }

        int newBufferSize = static_cast<int>(vertexCount * sizeof(glm::fvec4));
        if (newBufferSize <= currentBufferSize) {
            glNamedBufferSubData(vboID, 0, newBufferSize, &m_vertices[0].position.x);
        } else {
            glNamedBufferData(vboID, newBufferSize, &m_vertices[0].position.x, GL_STATIC_DRAW);
            currentBufferSize = newBufferSize;
        }
        vertexLock.unlock();
    }

    if (vertexCount > 0) {
        glBindVertexArray(vaoID);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
}

void Chunk::fillSunlight() {
    for (int z = 0; z < C_EXTEND; z++) {
        for (int x = 0; x < C_EXTEND; x++) {
            for (int y = C_HEIGHT-1; y >= 0 && m_blocks[linearizeCoord(x, y, z)].block == AIR; y++) {
                m_blocks[linearizeCoord(x, y, z)].sunLight = MAX_SUN_LIGHT;
            }
        }
    }
}

const st_block& Chunk::getBlock(int x, int y, int z) const {
    bool outOfBounds = (x < 0 || y < 0 || z < 0 || x >= C_EXTEND || z >= C_EXTEND || y >= C_HEIGHT);
    if (!outOfBounds)
        return m_blocks[linearizeCoord(x, y, z)];
    else if (z >= C_EXTEND && north != nullptr)
        return north->getBlock(x, y, z - C_EXTEND);
    else if (x >= C_EXTEND && east != nullptr)
        return east->getBlock(x - C_EXTEND, y, z);
    else if (z < 0 && south != nullptr)
        return south->getBlock(x, y, z + C_EXTEND);
    else if (x < 0 && west != nullptr)
        return west->getBlock(x + C_EXTEND, y, z);
    return AIR_BLOCK;
}
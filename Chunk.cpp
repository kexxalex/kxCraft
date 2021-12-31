/*
 * Chunk.cpp
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#include "Chunk.hpp"
#include <GL/glew.h>



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

    // Delete VAO and VBO
    if (vboID != 0) {
        glDeleteBuffers(1, &vboID);
    }
    if (vaoID != 0) {
        glDeleteVertexArrays(1, &vaoID);
    }

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

int Chunk::getCornerLight(int x, int y, int z) {
    const st_block& b111 = getBlock(x, y, z);
    const st_block& b101 = getBlock(x-1, y, z);
    const st_block& b110 = getBlock(x, y, z-1);
    const st_block& b100 = getBlock(x-1, y, z-1);

    const st_block& b011 = getBlock(x, y - 1, z);
    const st_block& b001 = getBlock(x-1, y - 1, z);
    const st_block& b010 = getBlock(x, y - 1, z-1);
    const st_block& b000 = getBlock(x-1, y - 1, z-1);

    int totalLight = (
            (b111.getLight() + b101.getLight() + b110.getLight() + b100.getLight() +
            b011.getLight() + b001.getLight() + b010.getLight() + b000.getLight())
    );

    return static_cast<int>(totalLight);
}

void Chunk::addFace(short ID, const glm::ivec3 &pos,
                    const glm::ivec3 &edgeA, const glm::ivec3 &edgeB) {

    glm::ivec3 p00(pos);
    glm::ivec3 p10(pos + edgeA);
    glm::ivec3 p01(pos + edgeB);
    glm::ivec3 p11(pos + edgeA + edgeB);
    m_vertices.emplace_back(pos, ID, getCornerLight(p00.x, p00.y, p00.z));
    m_vertices.emplace_back(pos + edgeA, getCornerLight(p11.x, p11.y, p11.z), getCornerLight(p10.x, p10.y, p10.z));
    m_vertices.emplace_back(pos + edgeB, 0, getCornerLight(p01.x, p01.y, p01.z));
}

void Chunk::addCube(int x, int y, int z, short block) {
    // Add top and bottom face
    const bool& connect = BLOCKS[block].connect;
    if (isTransparent(x, y + 1, z))
        addFace(BLOCKS[block].top,
                glm::ivec3(x, y + 1, z),
                glm::ivec3(0, 0, 1),
                glm::ivec3(1, 0, 0));
    if (isTransparent(x, y - 1, z))
        addFace(BLOCKS[block].bottom,
                glm::ivec3(x, y, z),
                glm::ivec3(1, 0, 0),
                glm::ivec3(0, 0, 1));


    // Add north and south face
    if (isTransparent(x, y, z + 1))
        addFace((connect && getBlock(x, y-1, z+1).ID == block) ? BLOCKS[block].top : BLOCKS[block].north,
                glm::ivec3(x, y, z + 1),
                glm::ivec3(1, 0, 0),
                glm::ivec3(0, 1, 0));
    if (isTransparent(x, y, z - 1))
        addFace((connect && getBlock(x, y-1, z-1).ID == block) ? BLOCKS[block].top : BLOCKS[block].south,
                glm::ivec3(x + 1, y, z),
                glm::ivec3(-1, 0, 0),
                glm::ivec3(0, 1, 0));


    // Add east and west face
    if (isTransparent(x - 1, y, z)) {
        addFace((connect && getBlock(x-1, y-1, z).ID == block) ? BLOCKS[block].top : BLOCKS[block].west,
                glm::ivec3(x, y, z),
                glm::ivec3(0, 0, 1),
                glm::ivec3(0, 1, 0));
    }
    if (isTransparent(x + 1, y, z))
        addFace((connect && getBlock(x+1, y-1, z).ID == block) ? BLOCKS[block].top : BLOCKS[block].east,
                glm::ivec3(x + 1, y, z + 1),
                glm::ivec3(0, 0, -1),
                glm::ivec3(0, 1, 0));
}

void Chunk::addCross(int x, int y, int z, short block) {
    bool t(getBlockAttributes(x, y + 1, z).transparent);
    bool b(getBlockAttributes(x, y - 1, z).transparent);
    bool n(getBlockAttributes(x, y, z + 1).transparent);
    bool e(getBlockAttributes(x + 1, y, z).transparent);
    bool s(getBlockAttributes(x, y, z - 1).transparent);
    bool w(getBlockAttributes(x - 1, y, z).transparent);

    if (!(t || b || n || e || s || w))
        return;

    short texID = BLOCKS[block].top;

    addFace(texID, glm::ivec3(x, y, z),
            glm::ivec3(1, 0, 1),
            glm::ivec3(0, 1, 0)
    );
    addFace(texID, glm::ivec3(x, y, z+1),
            glm::ivec3(1, 0, -1),
            glm::ivec3(0, 1, 0)
    );

    addFace(texID, glm::ivec3(x+1, y, z+1),
            glm::ivec3(-1, 0, -1),
            glm::ivec3(0, 1, 0)
    );
    addFace(texID, glm::ivec3(x+1, y, z),
            glm::ivec3(-1, 0, 1),
            glm::ivec3(0, 1, 0)
    );
}

void Chunk::update() {
    if (!chunkDestructionLock.try_lock())
        return;

    std::vector<glm::ivec3> updateBlocks;
    fillSunlight();
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
                        addCross(x, y, z, block.ID);
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

    glVertexArrayVertexBuffer(vaoID, 0, vboID, 0, sizeof(st_vertex));
    glVertexArrayVertexBuffer(vaoID, 1, vboID, 4, sizeof(st_vertex));

    glVertexArrayAttribFormat(vaoID, 0, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0);
    glVertexArrayAttribFormat(vaoID, 1, 1, GL_SHORT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(vaoID, 0);
    glEnableVertexArrayAttrib(vaoID, 1);
}

bool Chunk::chunkBufferUpdate() {
    if (vaoID == 0 && vboID == 0)
        initializeVertexArray();
    m_hasVertexUpdate = false;

    vertexCount = static_cast<int>(m_vertices.size());

    if (vertexCount == 0) {
        return false;
    }

    int newBufferSize = static_cast<int>(vertexCount * sizeof(st_vertex));
    if (newBufferSize <= currentBufferSize) {
        glNamedBufferSubData(vboID, 0, newBufferSize, &(m_vertices[0].position[0]));
    } else {
        glNamedBufferData(vboID, newBufferSize, &(m_vertices[0].position[0]), GL_STATIC_DRAW);
        currentBufferSize = newBufferSize;
    }

    return true;
}

void Chunk::render(int &availableChanges, Shader &shader) {
    if (!m_generated)
        return;

    if (availableChanges > 0 && m_hasVertexUpdate) {
        availableChanges--;
        if (!chunkBufferUpdate())
            return;
    }

    if (vertexCount > 0) {
        shader.setFloat3("CHUNK_POSITION", m_position);
        glBindVertexArray(vaoID);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
}

void Chunk::fillSunlight() {
    for (int z = 0; z < C_EXTEND; z++) {
        for (int x = 0; x < C_EXTEND; x++) {
            bool sunShaft = true;
            for (int y = C_HEIGHT-1; y >= 0; y--) {
                sunShaft = sunShaft && getBlockAttributes(x, y, z).transparent;
                m_blocks[linearizeCoord(x, y, z)].setSunLight(MAX_LIGHT * sunShaft);
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

    if (current.ID == BLOCK_ID::AIR && maxSunLight > current.getSunLight() + 1) {
        current.setSunLight(maxSunLight - 1);

        bool inBounds = (x > 0 && y > 0 & z > 0 && x+1 < C_EXTEND && z+1 < C_EXTEND && y+1 < C_HEIGHT);

        if (glm::abs(t.getSunLight() - current.getSunLight()) > 1 && inBounds)
        {
            updateBlocks.emplace_back(x, y+1, z);
        }
        if (glm::abs(b.getSunLight() - current.getSunLight()) > 1 && inBounds) {
            updateBlocks.emplace_back(x, y-1, z);
        }

        if (glm::abs(n.getSunLight() - current.getSunLight()) > 1) {
            if (inBounds)
                updateBlocks.emplace_back(x, y, z+1);
            else {
                if (m_north != nullptr)
                    m_north->m_needUpdate = true;
            }
        }
        if (glm::abs(e.getSunLight() - current.getSunLight()) > 1 && inBounds) {
            if (inBounds)
                updateBlocks.emplace_back(x+1, y, z);
            else {
                if (m_east != nullptr)
                    m_east->m_needUpdate = true;
            }
        }
        if (glm::abs(s.getSunLight() - current.getSunLight()) > 1 && inBounds) {
            if (inBounds)
                updateBlocks.emplace_back(x, y, z-1);
            else {
                if (m_south != nullptr)
                    m_south->m_needUpdate = true;
            }
        }
        if (glm::abs(w.getSunLight() - current.getSunLight()) > 1 && inBounds) {
            if (inBounds)
                updateBlocks.emplace_back(x-1, y, z);
            else {
                if (m_west != nullptr)
                    m_west->m_needUpdate = true;
            }
        }
    }
}

short Chunk::setBlock(int x, int y, int z, short block, bool forceUpdate) {
    short oldID = m_blocks[linearizeCoord(x, y, z)].ID;
    if (block != oldID) {
        m_blocks[linearizeCoord(x, y, z)].ID = block;
        if (!(block == DIRT || block == GRASS) && getBlock(x, y+1, z).ID == TALL_GRASS) {
            m_blocks[linearizeCoord(x, y+1, z)].ID = AIR;
        }
        if (forceUpdate) {
            update();
            chunkBufferUpdate();
        }

        if (z == C_EXTEND-1 && nullptr != m_north) {
            if (forceUpdate) {
                m_north->update();
                m_north->chunkBufferUpdate();
            }
            else
                m_north->m_needUpdate = true;
        }
        if (x == C_EXTEND-1 && nullptr != m_east) {
            if (forceUpdate) {
                m_east->update();
                m_east->chunkBufferUpdate();
            }
            else
                m_east->m_needUpdate = true;
        }
        if (z == 0 && nullptr != m_south) {
            if (forceUpdate) {
                m_south->update();
                m_south->chunkBufferUpdate();
            }
            else
                m_south->m_needUpdate = true;
        }
        if (x == 0 && nullptr != m_west) {
            if (forceUpdate) {
                m_west->update();
                m_west->chunkBufferUpdate();
            }
            else
                m_west->m_needUpdate = true;
        }
    }

    return oldID;
}

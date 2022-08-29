/*
 * Chunk.cpp
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#include "Chunk.hpp"
#include <GL/glew.h>
#include <fstream>
#include <filesystem>
#include <iostream>

struct st_chunk_disk_block {
    unsigned char ID;
    unsigned char count;
};

void Chunk::initialize(WorldGenerator *world_generator, unsigned int VBO, unsigned int OBO, unsigned int bufferOffset, Chunk *n, Chunk *e, Chunk *s, Chunk *w) {
    m_worldGenerator = world_generator;
    north = n;
    east = e;
    south = s;
    west = w;
    oboID = OBO;
    vboID = VBO;
    offset = bufferOffset;
    m_initialized = true;
}

Chunk::~Chunk() {
    chunkDestructionLock.lock();
    if (!m_generated)
        return;

    save();
    m_hasVertexUpdate = false;
    vertexCount = 0;
    m_vertices.clear();
}

void Chunk::save() {
    if (m_generated && m_hasUnsavedChanges) {
        std::ofstream chunkFile("./res/saves/default/" + chunkName);

        st_chunk_disk_block current{ m_blocks[0].ID, 0}; // count is off by one, since zero wouldn't be saved
        for (st_block *block_ptr = &m_blocks[1]; block_ptr - &m_blocks[0] < C_EXTEND*C_EXTEND*C_HEIGHT; block_ptr++) {
            if (block_ptr->ID == current.ID && current.count < 255)
                current.count++;
            else {
                chunkFile << current.ID << current.count;
                current.ID = block_ptr->ID;
                current.count = 0;
            }
        }
        chunkFile << current.ID << current.count;
        chunkFile.close();
    }
}

void Chunk::generate(int cx, int cz) {
    if (!chunkDestructionLock.try_lock())
        return;

    m_needUpdate = false;
    m_generated = false;
    m_position = glm::fvec3(cx, 0, cz);

    chunkName = std::to_string(cx) + "x" + std::to_string(cz);

    if (std::filesystem::exists("./res/saves/default/" + chunkName)) {
        std::ifstream chunkFile("./res/saves/default/" + chunkName, std::ios::binary);

        st_block *block_ptr = &m_blocks[0];
        st_chunk_disk_block current{ m_blocks[0].ID, 0};
        while (block_ptr - &m_blocks[0] < C_EXTEND*C_EXTEND*C_HEIGHT) {
            chunkFile.read(reinterpret_cast<char*>(&current.ID), 2);
            for (unsigned char i = 0; i <= current.count; i++) {
                (block_ptr++)->ID = current.ID;
            }
        }
        chunkFile.close();
    }
    else {
        m_worldGenerator->generate(cx, cz, &m_blocks[0]);
    }
    m_hasUnsavedChanges = true;
    m_generated = true;
    m_needUpdate = true;

    fillSunlight();

    north->m_needUpdate = true;
    east->m_needUpdate = true;
    south->m_needUpdate = true;
    west->m_needUpdate = true;

    chunkDestructionLock.unlock();
}

st_block& Chunk::getBlockRef(int x, int y, int z) {
    if (!m_initialized)
        return AIR_BLOCK;

    if (x >= 0 && y >= 0 && z >= 0 && x < C_EXTEND && z < C_EXTEND && y < C_HEIGHT)
        return m_blocks[linearizeCoord(x, y, z)];
    else if (z >= C_EXTEND)
        return north->getBlockRef(x, y, z - C_EXTEND);
    else if (x >= C_EXTEND)
        return east->getBlockRef(x - C_EXTEND, y, z);
    else if (z < 0)
        return south->getBlockRef(x, y, z + C_EXTEND);
    else if (x < 0)
        return west->getBlockRef(x + C_EXTEND, y, z);
    return AIR_BLOCK;
}

const st_block& Chunk::getBlock(int x, int y, int z) const {
    if (!m_initialized)
        return AIR_BLOCK;

    if (x >= 0 && y >= 0 && z >= 0 && x < C_EXTEND && z < C_EXTEND && y < C_HEIGHT)
        return m_blocks[linearizeCoord(x, y, z)];
    else if (z >= C_EXTEND)
        return north->getBlock(x, y, z - C_EXTEND);
    else if (x >= C_EXTEND)
        return east->getBlock(x - C_EXTEND, y, z);
    else if (z < 0)
        return south->getBlock(x, y, z + C_EXTEND);
    else if (x < 0)
        return west->getBlock(x + C_EXTEND, y, z);
    return AIR_BLOCK;
}

int Chunk::getCornerLight(int x, int y, int z, int dir) const {
    short b111 = getBlock(x, y, z).getLight();
    short b101 = getBlock(x-1, y, z).getLight();
    short b110 = getBlock(x, y, z-1).getLight();
    short b100 = getBlock(x-1, y, z-1).getLight();

    short b011 = getBlock(x, y - 1, z).getLight();
    short b001 = getBlock(x-1, y - 1, z).getLight();
    short b010 = getBlock(x, y - 1, z-1).getLight();
    short b000 = getBlock(x-1, y - 1, z-1).getLight();

    switch (dir) {
        case 0: // Top facing
            return b111 + b101 + b110 + b100;
        case 1: // Bottom facing
            return b011 + b001 + b010 + b000;
        case 2: // North facing
            return b111 + b101 + b011 + b001;
        case 3: // East facing
            return b111 + b110 + b011 + b010;
        case 4: // South facing
            return b110 + b100 + b010 + b000;
        case 5: // West facing
            return b101 + b100 + b001 + b000;
        default:
            return 0;
    }
}

void Chunk::addFace(short ID, const glm::ivec3 &pos, const glm::ivec3 &edgeA, const glm::ivec3 &edgeB, int dir){
    glm::ivec3 p00(pos);
    glm::ivec3 p10(pos + edgeA);
    glm::ivec3 p01(pos + edgeB);
    glm::ivec3 p11(pos + edgeA + edgeB);
    m_vertices.emplace_back(pos, ID, getCornerLight(p00.x, p00.y, p00.z, dir));
    m_vertices.emplace_back(pos + edgeA, getCornerLight(p11.x, p11.y, p11.z, dir), getCornerLight(p10.x, p10.y, p10.z, dir));
    m_vertices.emplace_back(pos + edgeB, 0, getCornerLight(p01.x, p01.y, p01.z, dir));
}

void Chunk::addCube(int x, int y, int z, short block) {
    // Add top and bottom face
    const bool& connect = BLOCKS[block].connect;
    const bool& translucent = BLOCKS[block].translucent;

    const Block& t(getBlockAttributes(x, y + 1, z));
    const Block& b(getBlockAttributes(x, y - 1, z));
    const Block& n(getBlockAttributes(x, y, z + 1));
    const Block& e(getBlockAttributes(x + 1, y, z));
    const Block& s(getBlockAttributes(x, y, z - 1));
    const Block& w(getBlockAttributes(x - 1, y, z));

    if (t.transparent || t.translucent || translucent)
        addFace(BLOCKS[block].top,
                glm::ivec3(x, y + 1, z),
                glm::ivec3(0, 0, 1),
                glm::ivec3(1, 0, 0), 0);
    if (b.transparent || b.translucent || translucent)
        addFace(BLOCKS[block].bottom,
                glm::ivec3(x, y, z),
                glm::ivec3(1, 0, 0),
                glm::ivec3(0, 0, 1), 1);


    // Add north and south face
    if (n.transparent || n.translucent || translucent)
        addFace((connect && getBlock(x, y-1, z+1).ID == block) ? BLOCKS[block].top : BLOCKS[block].north,
                glm::ivec3(x, y, z + 1),
                glm::ivec3(1, 0, 0),
                glm::ivec3(0, 1, 0), 2);
    if (s.transparent || s.translucent || translucent)
        addFace((connect && getBlock(x, y-1, z-1).ID == block) ? BLOCKS[block].top : BLOCKS[block].south,
                glm::ivec3(x + 1, y, z),
                glm::ivec3(-1, 0, 0),
                glm::ivec3(0, 1, 0), 4);


    // Add east and west face
    if (e.transparent || e.translucent || translucent)
        addFace((connect && getBlock(x+1, y-1, z).ID == block) ? BLOCKS[block].top : BLOCKS[block].east,
                glm::ivec3(x + 1, y, z + 1),
                glm::ivec3(0, 0, -1),
                glm::ivec3(0, 1, 0), 3);
    if (w.transparent || w.translucent || translucent) {
        addFace((connect && getBlock(x-1, y-1, z).ID == block) ? BLOCKS[block].top : BLOCKS[block].west,
                glm::ivec3(x, y, z),
                glm::ivec3(0, 0, 1),
                glm::ivec3(0, 1, 0), 5);
    }
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
            glm::ivec3(0, 1, 0),
            0
    );
    addFace(texID, glm::ivec3(x, y, z+1),
            glm::ivec3(1, 0, -1),
            glm::ivec3(0, 1, 0),
            0
    );
}

void Chunk::update() {
    if (!m_generated || !m_initialized || !chunkDestructionLock.try_lock())
        return;

    fillSunlight();
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

    m_needUpdate = false;
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

    m_hasVertexUpdate = true;
    chunkDestructionLock.unlock();
}

unsigned int Chunk::chunkBufferUpdate(int &availableChanges) {
    if (!m_generated)
        return 0;

    if (!m_hasVertexUpdate)
        return vertexCount;

    auto newVertexCount = static_cast<unsigned int>(m_vertices.size());
    m_hasVertexUpdate = false;

    if (newVertexCount == 0) {
        vertexCount = 0;
        return 0;
    }

    if (newVertexCount <= CHUNK_BASE_VERTEX_OFFSET) {
        // std::cout << "update chunk\n";
        vertexCount = newVertexCount;
        availableChanges--;
        glm::fvec3 chunkPosition = m_position * static_cast<float>(C_EXTEND);
        glNamedBufferSubData(oboID,
                             offset * sizeof(glm::fvec3),
                             sizeof(glm::fvec3),
                             &chunkPosition);

        auto segmentSize = static_cast<unsigned int>(newVertexCount * sizeof(st_vertex));
        glNamedBufferSubData(vboID,
                             offset * sizeof(st_vertex) * CHUNK_BASE_VERTEX_OFFSET,
                             segmentSize,
                             &(m_vertices[0].position[0])
        );
    }

    return vertexCount;
}

void Chunk::fillSunlight() {
    for (int z = 0; z < C_EXTEND; z++) {
        for (int x = 0; x < C_EXTEND; x++) {
            bool sunShaft = true;
            for (int y = C_HEIGHT-1; y >= 0; y--) {
                sunShaft = sunShaft && getBlockAttributes(x, y, z).translucent;
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

    if (BLOCKS[current.ID].translucent && maxSunLight > current.getSunLight() + 1) {
        current.setSunLight(maxSunLight - 1);

        if (abs(t.getSunLight() - current.getSunLight()) > 1) {
            updateBlocks.emplace_back(x, y + 1, z);
        }
        if (abs(b.getSunLight() - current.getSunLight()) > 1) {
            updateBlocks.emplace_back(x, y - 1, z);
        }
        if (abs(n.getSunLight() - current.getSunLight()) > 1) {
            updateBlocks.emplace_back(x, y, z + 1);
            if (z + 1 == C_EXTEND)
                north->m_needUpdate = true;
        }
        if (abs(e.getSunLight() - current.getSunLight()) > 1) {
            updateBlocks.emplace_back(x + 1, y, z);
            if (x + 1 == C_EXTEND)
                east->m_needUpdate = true;
        }
        if (abs(s.getSunLight() - current.getSunLight()) > 1) {
            updateBlocks.emplace_back(x, y, z - 1);
            if (z == -1)
                south->m_needUpdate = true;
        }
        if (abs(w.getSunLight() - current.getSunLight()) > 1) {
            updateBlocks.emplace_back(x - 1, y, z);
            if (x == -1)
                west->m_needUpdate = true;
        }
    }
}

short Chunk::setBlock(int x, int y, int z, short block, bool forceUpdate) {
    short oldID = m_blocks[linearizeCoord(x, y, z)].ID;
    if (block != oldID) {
        m_blocks[linearizeCoord(x, y, z)].ID = block;
        m_hasUnsavedChanges = true;

        const Block &top = getBlockAttributes(x, y + 1, z);
        if (!(block == DIRT || block == GRASS) && top.isPlant) {
            m_blocks[linearizeCoord(x, y + 1, z)].ID = AIR;
        }
        if (forceUpdate) {
            update();
        }

        if (z == C_EXTEND - 1) {
            if (forceUpdate) {
                north->update();
            }
            else
                north->m_needUpdate = true;
        }
        if (x == C_EXTEND-1) {
            if (forceUpdate) {
                east->update();
            }
            else
                east->m_needUpdate = true;
        }
        if (z == 0) {
            if (forceUpdate) {
                south->update();
            }
            else
                south->m_needUpdate = true;
        }
        if (x == 0) {
            if (forceUpdate) {
                west->update();
            }
            else
                west->m_needUpdate = true;
        }
    }

    return oldID;
}

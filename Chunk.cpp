/*
 * Chunk.cpp
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#include "Chunk.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <filesystem>
#include <iostream>

union st_chunk_disk_block
{
    constexpr st_chunk_disk_block(int ID = 0, int count = 0) : ID(ID), count(count) {}
    constexpr st_chunk_disk_block(const st_chunk_disk_block &) = default;

    char data[2];
    struct
    {
        unsigned char ID;
        unsigned char count;
    };
};

void Chunk::initialize(WorldGenerator *world_generator, unsigned int VBO, unsigned int OBO, unsigned int bufferOffset, Chunk *n, Chunk *e, Chunk *s, Chunk *w)
{
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

Chunk::~Chunk()
{
    chunkDestructionLock.lock();
    if (!m_generated)
        return;

    save();
    m_hasVertexUpdate = false;
    faceCount = 0;
    m_faces.clear();
}

void Chunk::save()
{
    if (m_generated && m_hasUnsavedChanges)
    {
        std::ofstream chunkFile(chunkPath, std::ios::binary);
        if (chunkFile)
        {
            std::vector<st_chunk_disk_block> compressed;
            compressed.reserve(C_EXTEND * C_EXTEND * C_HEIGHT);

            const st_block *blocks = &m_blocks[0][0][0];

            st_chunk_disk_block current(blocks[0].ID, 0); // count is off by one, since zero wouldn't be saved
            for (int k = 1; k < C_EXTEND * C_EXTEND * C_HEIGHT; k++)
            {
                if (blocks[k].ID == current.ID && current.count < 255)
                    current.count++;
                else
                {
                    compressed.emplace_back(current);
                    current.ID = blocks[k].ID;
                    current.count = 0;
                }
            }
            compressed.emplace_back(current);
            chunkFile.write(&compressed[0].data[0], compressed.size() * sizeof(st_chunk_disk_block));
        }
        chunkFile.close();
    }
}

void Chunk::generate(int cx, int cz)
{
    if (!chunkDestructionLock.try_lock())
        return;

    m_needUpdate = false;
    m_generated = false;
    m_position = glm::fvec3(cx, 0, cz);

    chunkName = std::to_string(cx) + "x" + std::to_string(cz);
    chunkPath = "./res/saves/default/" + chunkName;

    if (std::filesystem::exists(chunkPath))
    {
        std::ifstream chunkFile(chunkPath, std::ios::binary);
        st_block *const blocks = &m_blocks[0][0][0];
        int k = 0;
        while (k < C_EXTEND * C_EXTEND * C_HEIGHT)
        {
            st_chunk_disk_block current;
            chunkFile.read(current.data, 2);
            for (int i = 0; i <= (int)current.count; i++)
                blocks[k++].ID = current.ID;
        }
        chunkFile.close();
    }
    else
    {
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

st_block &Chunk::getBlockRef(int x, int y, int z)
{
    if (!m_initialized)
        return AIR_BLOCK;

    if (x >= 0 && y >= 0 && z >= 0 && x < C_EXTEND && z < C_EXTEND && y < C_HEIGHT)
        return m_blocks[x][z][y];
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

const st_block &Chunk::getBlock(int x, int y, int z) const
{
    if (!m_initialized)
        return AIR_BLOCK;

    if (x >= 0 && y >= 0 && z >= 0 && x < C_EXTEND && z < C_EXTEND && y < C_HEIGHT)
        return m_blocks[x][z][y];
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

short Chunk::getCornerLight(int x, int y, int z, FACE dir) const
{
    const short &b111 = getBlock(x, y, z).getLight();
    const short &b101 = getBlock(x - 1, y, z).getLight();
    const short &b110 = getBlock(x, y, z - 1).getLight();
    const short &b100 = getBlock(x - 1, y, z - 1).getLight();

    const short &b011 = getBlock(x, y - 1, z).getLight();
    const short &b001 = getBlock(x - 1, y - 1, z).getLight();
    const short &b010 = getBlock(x, y - 1, z - 1).getLight();
    const short &b000 = getBlock(x - 1, y - 1, z - 1).getLight();

    switch (dir)
    {
    case FACE::TOP: // Top facing
        return b111 + b101 + b110 + b100;
    case FACE::BOTTOM: // Bottom facing
        return b011 + b001 + b010 + b000;
    case FACE::NORTH: // North facing
        return b111 + b101 + b011 + b001;
    case FACE::EAST: // East facing
        return b111 + b110 + b011 + b010;
    case FACE::SOUTH: // South facing
        return b110 + b100 + b010 + b000;
    case FACE::WEST: // West facing
        return b101 + b100 + b001 + b000;
    default:
        return 0;
    }
}

void Chunk::addFace(short ID, const glm::ivec3 &pos, const glm::ivec3 &edgeA, const glm::ivec3 &edgeB, FACE mode)
{
    glm::ivec3 p00(pos);
    glm::ivec3 p10(pos + edgeA);
    glm::ivec3 p01(pos + edgeB);
    glm::ivec3 p11(pos + edgeA + edgeB);

    const uint8_t light[4] = {
        getCornerLight(p00.x, p00.y, p00.z, mode),
        getCornerLight(p10.x, p10.y, p10.z, mode),
        getCornerLight(p01.x, p01.y, p01.z, mode),
        getCornerLight(p11.x, p11.y, p11.z, mode)};

    m_faces.emplace_back(ID, pos, mode, light);
}

void Chunk::addCube(int x, int y, int z, short block)
{
    // Add top and bottom face
    const bool &connect = BLOCKS[block].connect;
    const bool &translucent = BLOCKS[block].translucent;

    const Block &t(getBlockAttributes(x, y + 1, z));
    const Block &b(getBlockAttributes(x, y - 1, z));
    const Block &n(getBlockAttributes(x, y, z + 1));
    const Block &e(getBlockAttributes(x + 1, y, z));
    const Block &s(getBlockAttributes(x, y, z - 1));
    const Block &w(getBlockAttributes(x - 1, y, z));

    if (t.transparent || t.translucent || translucent)
        addFace(BLOCKS[block].top,
                glm::ivec3(x, y + 1, z),
                glm::ivec3(0, 0, 1),
                glm::ivec3(1, 0, 0), FACE::TOP);
    if (b.transparent || b.translucent || translucent)
        addFace(BLOCKS[block].bottom,
                glm::ivec3(x, y, z),
                glm::ivec3(1, 0, 0),
                glm::ivec3(0, 0, 1), FACE::BOTTOM);

    // Add north and south face
    if (n.transparent || n.translucent || translucent)
        addFace((connect && getBlock(x, y - 1, z + 1).ID == block) ? BLOCKS[block].top : BLOCKS[block].north,
                glm::ivec3(x, y, z + 1),
                glm::ivec3(1, 0, 0),
                glm::ivec3(0, 1, 0), FACE::NORTH);
    if (s.transparent || s.translucent || translucent)
        addFace((connect && getBlock(x, y - 1, z - 1).ID == block) ? BLOCKS[block].top : BLOCKS[block].south,
                glm::ivec3(x + 1, y, z),
                glm::ivec3(-1, 0, 0),
                glm::ivec3(0, 1, 0), FACE::SOUTH);

    // Add east and west face
    if (e.transparent || e.translucent || translucent)
        addFace((connect && getBlock(x + 1, y - 1, z).ID == block) ? BLOCKS[block].top : BLOCKS[block].east,
                glm::ivec3(x + 1, y, z + 1),
                glm::ivec3(0, 0, -1),
                glm::ivec3(0, 1, 0), FACE::EAST);
    if (w.transparent || w.translucent || translucent)
    {
        addFace((connect && getBlock(x - 1, y - 1, z).ID == block) ? BLOCKS[block].top : BLOCKS[block].west,
                glm::ivec3(x, y, z),
                glm::ivec3(0, 0, 1),
                glm::ivec3(0, 1, 0), FACE::WEST);
    }
}

void Chunk::addCross(int x, int y, int z, short block)
{
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
            FACE::TOP);
    addFace(texID, glm::ivec3(x, y, z + 1),
            glm::ivec3(1, 0, -1),
            glm::ivec3(0, 1, 0),
            FACE::TOP);
}

void Chunk::update()
{
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
    m_faces.clear();

    for (int x = 0; x < C_EXTEND; x++)
    {
        for (int z = 0; z < C_EXTEND; z++)
        {
            for (int y = 0; y < C_HEIGHT; y++)
            {
                const st_block &block = m_blocks[x][z][y];
                if (block.ID == BLOCK_ID::AIR)
                    continue;

                switch (BLOCKS[block.ID].type)
                {
                case R_BLOCK:
                    addCube(x, y, z, block.ID);
                    break;
                    // case R_CROSS:
                    //     addCross(x, y, z, block.ID);
                    //     break;
                }
            }
        }
    }

    m_hasVertexUpdate = true;
    chunkDestructionLock.unlock();
}

unsigned int Chunk::chunkBufferUpdate(int &availableChanges)
{
    if (!m_generated)
        return 0;

    if (!m_hasVertexUpdate)
        return faceCount;

    auto newFaceCount = static_cast<uint32_t>(m_faces.size());
    m_hasVertexUpdate = false;

    if (newFaceCount == 0)
    {
        faceCount = 0;
        return 0;
    }

    //static uint32_t maxFaces = 0;

    //maxFaces = newFaceCount > maxFaces ? newFaceCount : maxFaces;
    //std::cout << glfwGetTime() << ": " << maxFaces << " -- " << offset * CHUNK_BASE_VERTEX_OFFSET * sizeof(st_face) / 1024.0/1024.0 << '\n';

    if (newFaceCount <= CHUNK_BASE_VERTEX_OFFSET)
    {
        faceCount = newFaceCount;
        availableChanges--;
        glm::fvec3 chunkPosition = m_position * C_EXTEND;
        glNamedBufferSubData(oboID,
                             offset * sizeof(glm::fvec3),
                             sizeof(glm::fvec3),
                             &chunkPosition);

        glNamedBufferSubData(
            vboID,
            offset * CHUNK_BASE_VERTEX_OFFSET * sizeof(st_face),
            newFaceCount * sizeof(st_face),
            m_faces.data());
    }

    return faceCount;
}

void Chunk::fillSunlight()
{
    for (int x = 0; x < C_EXTEND; x++)
    {
        for (int z = 0; z < C_EXTEND; z++)
        {
            bool sunShaft = true;
            for (int y = C_HEIGHT - 1; y >= 0; y--)
            {
                sunShaft = sunShaft && getBlockAttributes(x, y, z).translucent;
                m_blocks[x][z][y].setSunLight(MAX_LIGHT * sunShaft);
            }
        }
    }
}

void Chunk::updateBlockLight(int x, int y, int z, std::vector<glm::ivec3> &updateBlocks)
{
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
            glm::max(s.getSunLight(), w.getSunLight())));

    if (BLOCKS[current.ID].translucent && maxSunLight > current.getSunLight() + 1)
    {
        current.setSunLight(maxSunLight - 1);

        if (abs(t.getSunLight() - current.getSunLight()) > 1)
        {
            updateBlocks.emplace_back(x, y + 1, z);
        }
        if (abs(b.getSunLight() - current.getSunLight()) > 1)
        {
            updateBlocks.emplace_back(x, y - 1, z);
        }
        if (abs(n.getSunLight() - current.getSunLight()) > 1)
        {
            updateBlocks.emplace_back(x, y, z + 1);
            if (z + 1 == C_EXTEND)
                north->m_needUpdate = true;
        }
        if (abs(e.getSunLight() - current.getSunLight()) > 1)
        {
            updateBlocks.emplace_back(x + 1, y, z);
            if (x + 1 == C_EXTEND)
                east->m_needUpdate = true;
        }
        if (abs(s.getSunLight() - current.getSunLight()) > 1)
        {
            updateBlocks.emplace_back(x, y, z - 1);
            if (z == -1)
                south->m_needUpdate = true;
        }
        if (abs(w.getSunLight() - current.getSunLight()) > 1)
        {
            updateBlocks.emplace_back(x - 1, y, z);
            if (x == -1)
                west->m_needUpdate = true;
        }
    }
}

short Chunk::setBlock(int x, int y, int z, short block, bool forceUpdate)
{
    short oldID = m_blocks[x][z][y].ID;
    if (block != oldID)
    {
        m_blocks[x][z][y].ID = block;
        m_hasUnsavedChanges = true;

        const Block &top = getBlockAttributes(x, y + 1, z);
        if (!(block == DIRT || block == GRASS) && top.isPlant)
        {
            m_blocks[x][z][y].ID = AIR;
        }
        if (forceUpdate)
        {
            update();
        }

        if (z == C_EXTEND - 1)
        {
            if (forceUpdate)
            {
                north->update();
            }
            else
                north->m_needUpdate = true;
        }
        if (x == C_EXTEND - 1)
        {
            if (forceUpdate)
            {
                east->update();
            }
            else
                east->m_needUpdate = true;
        }
        if (z == 0)
        {
            if (forceUpdate)
            {
                south->update();
            }
            else
                south->m_needUpdate = true;
        }
        if (x == 0)
        {
            if (forceUpdate)
            {
                west->update();
            }
            else
                west->m_needUpdate = true;
        }
    }

    return oldID;
}

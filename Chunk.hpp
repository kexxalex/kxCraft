/*
 * Chunk.hpp
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#pragma once

#include <vector>
#include <mutex>

#include "Item.hpp"
#include "Block.hpp"
#include "WorldGenerator.hpp"
#include "Shader.hpp"
#include "3Dutils.hpp"

static constexpr unsigned int CHUNK_BASE_VERTEX_OFFSET = 12288; //16384/2*3;



class Chunk {
public:
    Chunk() = default;

    ~Chunk();

    void initialize(WorldGenerator *world_generator, unsigned int VBO, unsigned int OBO, unsigned int bufferOffset,
                    Chunk *north, Chunk *east, Chunk *south, Chunk *west);

    void update();
    void generate(int x, int z);
    void save();

    unsigned int chunkBufferUpdate(int &availableChanges);

    [[nodiscard]] constexpr unsigned int getVertexCount() const noexcept { return faceCount; }
    [[nodiscard]] constexpr bool needUpdate() const noexcept { return m_needUpdate; }
    [[nodiscard]] constexpr bool needBufferUpdate() const noexcept { return m_hasVertexUpdate; }
    [[nodiscard]] constexpr bool isGenerated() const noexcept { return m_generated; }
    [[nodiscard]] const Block& getBlockAttributes(int x, int y, int z) const { return BLOCKS[getBlock(x, y, z).ID]; }
    [[nodiscard]] inline const bool& isTransparent(int x, int y, int z) const {
        return getBlockAttributes(x, y, z).transparent;
    }

    [[nodiscard]] constexpr const glm::ivec3& getPosition() const noexcept { return m_position; }
    [[nodiscard]] constexpr glm::ivec2 getXZPosition() const noexcept { return {m_position.x, m_position.z}; }
    [[nodiscard]] const st_block& getBlock(int x, int y, int z) const;
    [[nodiscard]] inline const char* getChunkData() const { return reinterpret_cast<const char*>(m_blocks); }

    short setBlock(int x, int y, int z, short block = AIR, bool forceUpdate=false);

private:
    st_block m_blocks[C_EXTEND][C_EXTEND][C_HEIGHT];

    bool m_initialized{ false };
    bool m_needUpdate{ false };
    bool m_generated{ false };
    bool m_hasUnsavedChanges{ false };

    [[nodiscard]] st_block& getBlockRef(int x, int y, int z);
    [[nodiscard]] inline short getCornerLight(int x, int y, int z, FACE dir) const;

    void addFace(short ID, const glm::ivec3 &pos, const glm::ivec3 &edgeA, const glm::ivec3 &edgeB, FACE mode);
    void addCube(int x, int y, int z, short block);
    void addCross(int x, int y, int z, short block);
    void fillSunlight();
    void updateBlockLight(int x, int y, int z, std::vector<glm::ivec3> &updateBlocks);

    glm::ivec3 m_position{0, 0, 0};
    WorldGenerator *m_worldGenerator{ nullptr };
    Chunk *north{ nullptr }, *east{ nullptr }, *south{ nullptr }, *west{ nullptr };

    bool m_hasVertexUpdate{ true };
    unsigned int offset{ 0 };

    unsigned int faceCount{ 0 };
    unsigned int oboID{ 0 };
    unsigned int vboID{ 0 };

    std::vector<st_face> m_faces;
    std::string chunkName{};
    std::string chunkPath{};

    std::mutex chunkDestructionLock;
};


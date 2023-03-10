//
// Created by kexx on 03.01.22.
//

#include "HUD.hpp"
#include <GL/glew.h>
#include "Block.hpp"
#include <iostream>

HUD::HUD()
{
    glCreateVertexArrays(1, &vaoID);
    glCreateBuffers(2, vboID);

    glVertexArrayVertexBuffer(vaoID, 0, vboID[0], 0, sizeof(st_face));
    glVertexArrayAttribIFormat(vaoID, 0, 4, GL_UNSIGNED_BYTE, 0);

    glVertexArrayVertexBuffer(vaoID, 1, vboID[0], 4, sizeof(st_face));
    glVertexArrayAttribIFormat(vaoID, 1, 1, GL_SHORT, 0);

    glVertexArrayVertexBuffer(vaoID, 2, vboID[1], 0, sizeof(glm::fvec3));
    glVertexArrayAttribFormat(vaoID, 2, 3, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(vaoID, 0);
    glEnableVertexArrayAttrib(vaoID, 1);
    glEnableVertexArrayAttrib(vaoID, 2);
}

void HUD::end()
{
    if (faces.size() > currentBufferSize)
    {
        currentBufferSize = faces.size();
        glNamedBufferData(vboID[0], static_cast<GLsizeiptr>(sizeof(st_face) * faces.size()), &faces[0].position[0],
                          GL_STREAM_DRAW);
        glNamedBufferData(vboID[1], static_cast<GLsizeiptr>(sizeof(glm::fvec3) * offsets.size()), &offsets[0].x,
                          GL_STREAM_DRAW);
    }
    else
    {
        glNamedBufferSubData(vboID[0], 0, static_cast<GLsizeiptr>(sizeof(st_face) * faces.size()), &faces[0].position[0]);
        glNamedBufferSubData(vboID[1], 0, static_cast<GLsizeiptr>(sizeof(glm::fvec3) * offsets.size()), &offsets[0].x);
    }

    glBindVertexArray(vaoID);
    glDrawArrays(GL_POINTS, 0, faces.size());
}

void HUD::addBlock(const glm::fvec3 &position, uint8_t block, uint8_t brightness)
{
    const uint8_t light[4] = {brightness, brightness, brightness, brightness};
    for (int i = 0; i < 18; i++)
        offsets.push_back(position);

    // Add top and bottom face
    faces.emplace_back(
        BLOCKS[block].top,
        glm::ivec3(0, 1, 0),
        FACE::TOP,
        light
    );
    faces.emplace_back(
        BLOCKS[block].bottom,
        glm::ivec3(0, 0, 0),
        FACE::BOTTOM,
        light
    );

    // Add north and south face
    faces.emplace_back(
        BLOCKS[block].north,
        glm::ivec3(0, 0, 1),
        FACE::NORTH,
        light
    );
    faces.emplace_back(
        BLOCKS[block].south,
        glm::ivec3(1, 0, 0),
        FACE::SOUTH,
        light
    );

    // Add east and west face
    faces.emplace_back(
        BLOCKS[block].east,
        glm::ivec3(1, 0, 1),
        FACE::EAST,
        light
    );
    faces.emplace_back(
        BLOCKS[block].west,
        glm::ivec3(0, 0, 0),
        FACE::WEST,
        light
    );
}

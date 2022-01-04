//
// Created by kexx on 03.01.22.
//

#include "HUD.hpp"
#include <GL/glew.h>
#include "Block.hpp"
#include <iostream>



HUD::HUD() {
    glCreateVertexArrays(1, &vaoID);
    glCreateBuffers(2, vboID);

    std::cout << vboID[0] << vboID[1] << vaoID << std::endl;

    glVertexArrayVertexBuffer(vaoID, 0, vboID[0], 0, sizeof(st_vertex));
    glVertexArrayAttribFormat(vaoID, 0, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0);

    glVertexArrayVertexBuffer(vaoID, 1, vboID[0], 4, sizeof(st_vertex));
    glVertexArrayAttribFormat(vaoID, 1, 1, GL_SHORT, GL_FALSE, 0);

    glVertexArrayVertexBuffer(vaoID, 2, vboID[1], 0, sizeof(glm::fvec3));
    glVertexArrayAttribFormat(vaoID, 2, 3, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(vaoID, 0);
    glEnableVertexArrayAttrib(vaoID, 1);
    glEnableVertexArrayAttrib(vaoID, 2);
}

void HUD::end() {
    if (vertices.size() > currentBufferSize) {
        currentBufferSize = vertices.size();
        glNamedBufferData(vboID[0], static_cast<GLsizei>(sizeof(st_vertex) * vertices.size()), &vertices[0].position[0],
                          GL_STREAM_DRAW);
        glNamedBufferData(vboID[1], static_cast<GLsizei>(sizeof(glm::fvec3) * offsets.size()), &offsets[0].x,
                          GL_STREAM_DRAW);
    }
    else {
        glNamedBufferSubData(vboID[0], 0, static_cast<GLsizei>(sizeof(st_vertex) * vertices.size()), &vertices[0].position[0]);
        glNamedBufferSubData(vboID[1], 0, static_cast<GLsizei>(sizeof(glm::fvec3) * offsets.size()), &offsets[0].x);
    }

    glBindVertexArray(vaoID);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}

void HUD::addBlock(const glm::fvec3 &position, unsigned char block, short brightness) {
    const short light[4] = {brightness, brightness, brightness, brightness};
    for (int i=0; i<18; i++)
        offsets.push_back(position);

    // Add top and bottom face
    addFace(BLOCKS[block].top,
            glm::ivec3(0, 1, 0),
            glm::ivec3(0, 0, 1),
            glm::ivec3(1, 0, 0),
            light, vertices);
    addFace(BLOCKS[block].bottom,
            glm::ivec3 ( 0, 0, 0),
            glm::ivec3(1, 0, 0),
            glm::ivec3(0, 0, 1),
            light, vertices);

    // Add north and south face
    addFace(BLOCKS[block].north,
            glm::ivec3(0, 0, 1),
            glm::ivec3(1, 0, 0),
            glm::ivec3(0, 1, 0),
            light, vertices);
    addFace(BLOCKS[block].south,
            glm::ivec3(1, 0, 0),
            glm::ivec3(-1, 0, 0),
            glm::ivec3(0, 1, 0),
            light, vertices);


    // Add east and west face
    addFace(BLOCKS[block].east,
            glm::ivec3(1, 0, 1),
            glm::ivec3(0, 0, -1),
            glm::ivec3(0, 1, 0),
            light, vertices);
    addFace(BLOCKS[block].west,
            glm::ivec3(0, 0, 0),
            glm::ivec3(0, 0, 1),
            glm::ivec3(0, 1, 0),
            light, vertices);
}


//
// Created by kexx on 03.01.22.
//

#pragma once
#include "3Dutils.hpp"


class HUD {
public:
    HUD();

    void addBlock(const glm::fvec3 &position, unsigned char block, short brightness);
    inline void addBlockScreenSpace(const glm::mat3x3 &TBN, const glm::fvec3 &eye, const glm::fvec3 &screen, unsigned char block, short brightness) {
        addBlock(eye - TBN[0] * screen.x * 0.03125f * screen.z + TBN[1] * screen.y * 0.03125f * screen.z + TBN[2] * screen.z, block, brightness);
    }
    void begin() {
        vertices.clear();
        offsets.clear();
    }
    void end();

private:
    unsigned int vaoID{ 0 };
    unsigned int vboID[2]{ 0, 0 };

    int currentBufferSize{ 0 };

    std::vector<st_vertex> vertices;
    std::vector<glm::fvec3> offsets;
};

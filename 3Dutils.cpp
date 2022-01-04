//
// Created by kexx on 04.01.22.
//
#include "3Dutils.hpp"


void addFace(short ID, const glm::ivec3 &pos, const glm::ivec3 &edgeA, const glm::ivec3 &edgeB,
             const short (&light)[4], std::vector<st_vertex> &vertices) {
    vertices.emplace_back(pos, ID, light[0]);
    vertices.emplace_back(pos + edgeA, light[1], light[2]);
    vertices.emplace_back(pos + edgeB, 0, light[3]);
}
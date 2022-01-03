//
// Created by kexx on 03.01.22.
//

#include "HUD.hpp"
#include <GL/glew.h>

HUD::HUD() {
    glCreateVertexArrays(1, &vaoID);
    glCreateBuffers(1, &vboID);
}

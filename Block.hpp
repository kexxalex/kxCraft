/*
 * Block.h
 *
 *  Created on: Dec 26, 2021
 *      Author: kexx
 */

#pragma once

enum TEX_ID {
    T_AIR = 0,
    T_GRASS_TOP = 1, T_STONE, T_DIRT, T_GRASS_SIDE, T_WOOD_PLANKS,
    T_COBBLESTONE = 17, T_BEDROCK,
    T_GOLD_ORE = 33, T_IRON_ORE, T_COAL_ROE
};

enum BLOCK_ID {
    AIR, GRASS, STONE, DIRT, WOOD_PLANKS
};

enum RENDER_TYPE { R_BLOCK, R_CROSS };

class Block {
public:
    Block() = default;

    Block(const char *name, int texture, bool transparent = false, bool collision = true, bool connect = false,
          RENDER_TYPE type = R_BLOCK)
            : Block(name, texture, texture, texture, transparent, collision, connect, type) {}

    Block(const char *name, int top, int bottom, int side,
          bool transparent = false, bool collision = true, bool connect = false, RENDER_TYPE type = R_BLOCK)
            : Block(name, transparent, collision, connect, top, bottom, side, side, side, side, type) {}

    Block(const char *name, bool transparent, bool collision, bool connect,
          int top, int bottom, int north, int east, int south, int west,
          RENDER_TYPE type = R_BLOCK)
            : name(name), transparent(transparent), connect(connect), collision(collision),
              top(top - 1), bottom(bottom - 1), north(north - 1), east(east - 1), south(south - 1), west(west - 1),
              type(type) {}

    const char *name{"Unnamed"};
    const bool transparent{false};
    const bool collision{true};
    const bool connect{false};
    const RENDER_TYPE type{ R_BLOCK };

    const unsigned char top{0};
    const unsigned char bottom{0};
    const unsigned char north{0};
    const unsigned char east{0};
    const unsigned char south{0};
    const unsigned char west{0};
};


static Block BLOCKS[]{
        {"Air",         T_AIR,       true,   false},
        {"Grass",       T_GRASS_TOP, T_DIRT, T_GRASS_SIDE, false, true, true},
        {"Stone",       T_STONE},
        {"Dirt",        T_DIRT},
        {"Wood Planks", T_WOOD_PLANKS},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {},
        {"Cobblestone", T_COBBLESTONE},
        {"Bedrock",     T_BEDROCK}
};

/*
 * Block.h
 *
 *  Created on: Dec 26, 2021
 *      Author: kexx
 */

#ifndef BLOCK_HPP_
#define BLOCK_HPP_

enum TEX_ID {
    T_AIR = 0,
    T_GRASS_TOP = 1, T_STONE, T_DIRT, T_GRASS_SIDE, T_WOOD_PLANKS,
    T_COBBLESTONE = 17, T_BEDROCK,
    T_GOLD_ORE = 33, T_IRON_ORE, T_COAL_ROE
};

enum RENDER_TYPE { R_BLOCK, R_CROSS };

struct st_block_prop {
    st_block_prop() = default;

    st_block_prop(const char *name, int texture, bool transparent = false, bool connect = false, RENDER_TYPE type=R_BLOCK)
            : st_block_prop(name, texture, texture, texture, transparent, connect, type) {}

    st_block_prop(const char *name, int top, int bottom, int side, bool transparent = false, bool connect = false, RENDER_TYPE type=R_BLOCK)
            : st_block_prop(name, transparent, connect, top, bottom, side, side, side, side, type) {}

    st_block_prop(const char *name, bool transparent, bool connect,
                  int top, int bottom, int north, int east, int south, int west,
                  RENDER_TYPE type = R_BLOCK)
            : name(name), transparent(transparent), connect(connect),
              top(top - 1), bottom(bottom - 1), north(north - 1), east(east - 1), south(south - 1), west(west - 1),
              type(type) {}

    const char *name{"Unnamed"};
    bool transparent{false};
    bool connect{ false };
    RENDER_TYPE type{ R_BLOCK };

    unsigned char top{0};
    unsigned char bottom{0};
    unsigned char north{0};
    unsigned char east{0};
    unsigned char south{0};
    unsigned char west{0};
};


static st_block_prop BLOCK_PROP[256]{
        {"Air",         T_AIR,       true},
        {"Grass",       T_GRASS_TOP, T_DIRT, T_GRASS_SIDE, false, true},
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

enum BLOCK_ID {
    AIR, GRASS, STONE, DIRT, WOOD_PLANKS
};

#endif /* BLOCK_HPP_ */

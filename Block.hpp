/*
 * Block.h
 *
 *  Created on: Dec 26, 2021
 *      Author: kexx
 */

#pragma once

enum TEX_ID {
    T_AIR = 0,
    T_GRASS_TOP = 1, T_GRASS = 1, T_SNOW_TOP, T_SNOW = 2, T_SAND, T_DIRT, T_STONE, T_BEDROCK,
    T_OAK_LOG_SIDE, T_OAK_LOG_TOP, T_OAK_LEAVES,
    T_COAL_ORE, T_IRON_ORE, T_GOLD_ORE, T_REDSTONE_ORE, T_LAPIS_ORE, T_DIAMOND_ORE, T_EMERALD_ORE,
    T_GRASS_SIDE, T_SNOW_SIDE, T_OAK_PLANKS, T_COBBLESTONE, T_STONEBRICKS, T_BRICKS,
    T_SPRUCE_LOG_SIDE, T_SPRUCE_LOG_TOP, T_SPRUCE_LEAVES,
    T_COAL_BLOCK, T_IRON_BLOCK, T_GOLD_BLOCK, T_REDSTONE_BLOCK, T_LAPIS_BLOCK, T_DIAMOND_BLOCK, T_EMERALD_BLOCK,
    T_GLASS, T_OVEN_FRONT_ON, T_OVEN_FRONT_OFF, T_OVEN_BACK, T_OVEN_TOP,

    T_TALL_GRASS = 241, T_TULIP, T_SUNFLOWER, T_LILY,
    T_SELECTION = 256
};

enum RENDER_TYPE { R_BLOCK, R_CROSS };

class Block {
public:
    Block() = default;

    Block(const char *name, int texture,
          bool transparent = false, bool collision = true, bool connect = false, bool translucent = false,
          bool isPlant = false,
          RENDER_TYPE type = R_BLOCK)
            : Block(name, texture, texture, texture, transparent, collision, connect, translucent, isPlant, type) {}

    Block(const char *name, int top, int bottom, int side,
          bool transparent = false, bool collision = true, bool connect = false, bool translucent = false,
          bool isPlant = false,
          RENDER_TYPE type = R_BLOCK)
            : Block(name, transparent, collision, connect, translucent, isPlant, top, bottom, side, side, side, side,
                    type) {}

    Block(const char *name, bool transparent, bool collision, bool connect, bool translucent, bool isPlant,
          int top, int bottom, int north, int east, int south, int west,
          RENDER_TYPE type = R_BLOCK)
            : name(name), transparent(transparent), connect(connect), collision(collision), translucent(translucent),
              isPlant(isPlant),
              top(top - 1), bottom(bottom - 1), north(north - 1), east(east - 1), south(south - 1), west(west - 1),
              type(type) {}

    const char *name{"Unnamed"};
    const bool transparent{false};
    const bool collision{true};
    const bool connect{false};
    const bool translucent{false};
    const bool isPlant{false};
    const RENDER_TYPE type{R_BLOCK};

    const unsigned char top{0};
    const unsigned char bottom{0};
    const unsigned char north{0};
    const unsigned char east{0};
    const unsigned char south{0};
    const unsigned char west{0};
};


static Block BLOCKS[256]{
        {"Air",           T_AIR,         true,          false,        false, true},
        {"Grass",         T_GRASS_TOP,   T_DIRT,        T_GRASS_SIDE, false, true, true},
        {"Stone",         T_STONE},
        {"Dirt",          T_DIRT},
        {"Wood Planks",   T_OAK_PLANKS},
        {"Cobblestone",   T_COBBLESTONE},
        {"Coal Ore",      T_COAL_ORE},
        {"Iron Ore",      T_IRON_ORE},
        {"Gold Ore",      T_GOLD_ORE},
        {"Lapis Lazuli",  T_LAPIS_ORE},
        {"Red Stone Ore", T_REDSTONE_ORE},
        {"Diamant Ore",   T_DIAMOND_ORE},
        {"Emerald Ore",   T_EMERALD_ORE},
        {"Bedrock",       T_BEDROCK},
        {"Tall Grass",    T_TALL_GRASS,  true,          false,        false, true, true, R_CROSS},
        {"Tulip",         T_TULIP,       true,          false,        false, true, true, R_CROSS},
        {"Sunflower",     T_SUNFLOWER,   true,          false,        false, true, true, R_CROSS},
        {"Lily",          T_LILY,        true,          false,        false, true, true, R_CROSS},
        {"Oak Log",       T_OAK_LOG_TOP, T_OAK_LOG_TOP, T_OAK_LOG_SIDE},
        {"Oak Leaves",    T_OAK_LEAVES,  true,          true},
        {"Glass",         T_GLASS,       true,          true,         false, true},

        {"Selection",     T_SELECTION,   true}

};

enum BLOCK_ID {
    AIR,
    GRASS,
    STONE,
    DIRT,
    WOOD_PLANKS,
    COBBLESTONE,
    COAL_ORE,
    IRON_ORE,
    GOLD_ORE,
    LAPIS_ORE,
    REDSTONE_ORE,
    DIAMOND_ORE,
    EMERALD_ORE,
    BEDROCK,
    TALL_GRASS,
    TULIP,
    SUNFLOWER,
    LILY,
    OAK_LOG,
    OAK_LEAVES,
    GLASS,
    SELECTION
};

static const unsigned char BUILDABLE[] {
    GRASS,
    DIRT,
    STONE,
    COBBLESTONE,
    WOOD_PLANKS,
    GLASS,
    BEDROCK,
    OAK_LOG,
    OAK_LEAVES};
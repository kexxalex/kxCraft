cmake_minimum_required(VERSION 3.21)
project(kxCraft)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp
        ShaderManager.cpp ShaderManager.hpp
        Shader.cpp Shader.hpp
        Chunk.cpp Chunk.hpp
        World.cpp World.hpp
        WorldGenerator.cpp WorldGenerator.hpp
        BMP.cpp BMP.hpp
        Texture.cpp Texture.hpp
        TextureManager.cpp TextureManager.hpp
        Item.hpp Block.hpp
        Player.cpp Player.hpp
        HUD.cpp HUD.hpp
        3Dutils.hpp 3Dutils.cpp)

add_executable(kxCraft ${SOURCE_FILES})
target_link_libraries(kxCraft -lGLEW -lglfw -lGL -lpthread -lnoise)

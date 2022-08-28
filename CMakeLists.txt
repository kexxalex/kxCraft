cmake_minimum_required(VERSION 3.21)
project(kxCraft)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp
        ShaderManager.cpp
        Shader.cpp
        Chunk.cpp
        World.cpp
        WorldGenerator.cpp
        BMP.cpp
        Texture.cpp
        TextureManager.cpp
        Player.cpp
        HUD.cpp 3Dutils.cpp)

add_executable(kxCraft ${SOURCE_FILES})
target_link_libraries(kxCraft -lGLEW -lglfw -lGL -lpthread -lnoise)

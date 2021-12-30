# kxCraft

kxCraft is a C++ based minecraft clone using modern OpenGL (>= 4.2 required) with DSA and one extra thread for world generation.

## Feature list
- [x] Multithreaded world loader (only stable for one thread)
- [x] GLSL Vertex, Fragment and Geometry Shader
- [x] Angle view culling
- [x] Cave and mountain generation
- [x] Lighting system
- [x] Collision detection
- [x] Basic player movement
- [ ] Generate decoration like grass, flowers and trees
- [ ] Build and destroy blocks
- [ ] PBR Shader

## Currently working on
 - Generate decoration like grass, flowers and trees
 - Custom textures



## Compiling for Linux

### 1. Required libraries (for *Linux* I'm using Feren OS))
   - libglew-dev
   - libglfw3-dev
   - libglm-dev
   - libnoise-dev
   
### 2. Compile procedure
 1. Open a Terminal inside the **kxCraft** folder 
 2. Compile the source code
 
    
    c++ *.cpp -o kxCraft -pthread -O3 -lGLEW -lglfw -lGL -lnoise

## Compiling for Windows x64
**Assuming you have MS Visual Studio and clang++.**
<br>The required headers for this scenario are already included in the **kxCraft\windows** directory.
 1. Start a Command Line or Powershell in the kxCraft directory.
 2. Now you can use the following instruction to compile the source code to **kxCraft.exe** (64 bit)
 

    clang++ *.cpp -o kxCraft.exe -O3 -I./windows/include/ -L./windows/lib/ -lglew -lglfw3dll -lOpenGL32 -LibNoise64

## Screenshot
![kxCraft Hello](https://github.com/kexxalex/kxCraft/blob/master/kxCraft-Hello.png)


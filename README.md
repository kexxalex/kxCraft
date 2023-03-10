# kxCraft

kxCraft is a C++ based minecraft clone using OpenGL 4.5 with DSA and one (or more) extra threads for world generation.

## Feature list
- [x] Multithreaded world loader
- [x] GLSL Vertex, Fragment and Geometry Shader
- [x] Angle view culling using loop with __*glDrawArraysInstancedBaseInstance*__
- [x] Highly optimized memory management
  - Compact vertex struct (6Bytes) which increases FPS (up to 10%) and decreases memory usage
  - Geometry Shader calculates normals, UVs and the fourth vertex for each quad
  - Using RenderDistance 48 occupies <1GB and RD 16 just 230MB of video memory for the chunks itself <br> (+ some KB for indirect and offset buffer)
- [x] Cave and mountain generation with resources (Coal, Iron, Gold and Diamond)
- Lighting system
- [x] Collision detection
- [x] Basic player movement
- [x] Generate decoration like grass, flowers and trees
- [x] Build and destroy blocks
- [x] 3D HUD
- [ ] PBR Shader

## Currently working on
1. Lighting and PBR
2. Custom textures




## Linux

### 1. Required libraries for *Linux*
   - glew
   - glfw3
   - glm
   - noise
   
### 2. Compile procedure
 1. Open a Terminal inside the **kxCraft** folder 
 2. Compile the source code <br>
`g++ *.cpp -std=gnu++20 -o kxCraft -pthread -O3 -lGLEW -lglfw -lGL -lnoise`

## Screenshot
![kxCraft Hello](https://github.com/kexxalex/kxCraft/blob/master/kxCraft-Hello.png)


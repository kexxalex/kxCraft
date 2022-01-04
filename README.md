# kxCraft

kxCraft is a C++ based minecraft clone using OpenGL 4.5 with DSA and one (or more) extra threads for world generation.

## Feature list
- [x] Multithreaded world loader
- [x] GLSL Vertex, Fragment and Geometry Shader
- [x] Angle view culling using __*glMultiDrawIndirect*__ or loop with __*glDrawArraysInstancedBaseInstance*__
  - Switch by holding ALT-key. Default is the loop because most systems GL call overhead is neglectable
- [x] Highly optimized memory management
  - Compact vertex struct (6Bytes) which increases FPS (up to 10%) and decreases memory usage
  - Geometry Shader calculates normals, UVs and the fourth vertex for each quad 
  - Using RenderDistance 48 occupies <2GB and RD 16 just 230MB of video memory for the chunks itself <br> (+ some KB for indirect and offset buffer)
- [x] Cave and mountain generation
- [x] Lighting system
- [x] Collision detection
- [x] Basic player movement
- [x] Generate decoration like grass and trees
- [x] Build and destroy blocks
- [x] 3D HUD
- [ ] PBR Shader

## Currently working on
1. Menus
2. Optimizing __*TextureManager*__ class
3. Custom textures




## Compiling for Linux

### 1. Required libraries for *Linux*
   - libglew-dev
   - libglfw3-dev
   - libglm-dev
   - libnoise-dev
   
### 2. Compile procedure
 1. Open a Terminal inside the **kxCraft** folder 
 2. Compile the source code <br>
`c++ *.cpp -std=c++17 -o kxCraft -pthread -O3 -lGLEW -lglfw -lGL -lnoise`

## Compiling for Windows x64
**Assuming you have MS Visual Studio and clang++.** For other compilers you probably know what to do, by looking at this statement.
<br>The required headers for this scenario are already included in the **kxCraft\windows** directory.
 1. Start a Command Line or Powershell in the kxCraft folder.
 2. Now you can use the following instruction to compile the source code to **kxCraft.exe** (64 bit) <br>
`clang++ *.cpp -std=c++17  -o kxCraft.exe -O3 -I./windows/include/ -L./windows/lib/ -lglew -lglfw3dll -lOpenGL32 -LibNoise64`

## Screenshot
![kxCraft Hello](https://github.com/kexxalex/kxCraft/blob/master/kxCraft-Hello.png)


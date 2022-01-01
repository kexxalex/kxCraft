/*
 * main.cpp
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */




#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <thread>
#include <iostream>
#include "World.hpp"
#include "Player.hpp"
#include "ShaderManager.hpp"
#include "TextureManager.hpp"


static GLFWwindow *WINDOW = nullptr;
static ShaderManager SHADER_MANAGER;
static TextureManager TEXTURE_MANAGER;
static int WIDTH = 1600;
static int HEIGHT = 900;
static constexpr bool VSYNC = false;
static constexpr int THREAD_COUNT = 1;
static bool FIRST_UPDATE[THREAD_COUNT] = { false };
static World WORLD;
static Player PLAYER;



void loadShader() {
    Shader& shader = SHADER_MANAGER.getShader("./res/shader/terrain");
    shader.Bind();
    shader.setInt("DIFFUSE", 0);
    shader.setFloat("INV_RENDER_DIST", 1.0f / WORLD.getRenderDistance());
    shader.setBool("DISTANCE_CULLING", false);
}


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        SHADER_MANAGER.reloadAll();
        loadShader();
    }
    if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
        WORLD.reloadCurrentChunk();
    }
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}

bool initGLWindow() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    WINDOW = glfwCreateWindow(WIDTH, HEIGHT, "kxCraft", nullptr, nullptr);
    if (!WINDOW) {
        std::cerr << "Couldn't create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwSetKeyCallback(WINDOW, key_callback);
    glfwSetWindowSizeCallback(WINDOW, window_size_callback);
    glfwMakeContextCurrent(WINDOW);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Couldn't initialize GLEW" << std::endl;
        glfwTerminate();
        return false;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    return true;
}

void worldUpdaterThread(int thrID) {
    WORLD.update(thrID);
    while (WORLD.isActive()) {
        WORLD.update(thrID);
        FIRST_UPDATE[thrID] = true;
    }
}


void render() {
    static Shader &terrainShader = SHADER_MANAGER.getShader("./res/shader/terrain");

    static double lastFPS = glfwGetTime();
    static int frames{0};
    static glm::fvec3 up(0, 1, 0);
    static glm::fmat4x4 proj = glm::perspective(
            glm::radians(65.0f),
            (float)WIDTH / (float)HEIGHT,
            0.03f, 1024.0f);

    static double lastFrame = glfwGetTime();
    double time = glfwGetTime();
    double dTime = time - lastFrame;
    lastFrame = time;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PLAYER.update(WINDOW, time, dTime);

    glm::fmat4x4 view = glm::lookAt(PLAYER.getEyePosition(), PLAYER.getEyePosition() + PLAYER.getDirection(), up);
    glm::fmat4x4 MVP = proj * view;

    terrainShader.Bind();
    terrainShader.setMatrixFloat4("MVP", MVP);
    terrainShader.setFloat3("PLAYER_POSITION", PLAYER.getEyePosition());
    terrainShader.setFloat("TIME", (float)time);

    WORLD.render(terrainShader);

    glfwSwapBuffers(WINDOW);
    frames++;

    if (time - lastFPS >= 1.0) {
        glfwSetWindowTitle(WINDOW, ("kxCraft - " + std::to_string(frames)).c_str());
        frames = 0;
        lastFPS = time;
    }

    glfwSwapInterval(VSYNC);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Couldn't initialize GLFW" << std::endl;
        return 1;
    }

    if (!initGLWindow()) {
        return 2;
    }

    std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

    glClearColor(0.75, 0.9, 1.0, 1.0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    SHADER_MANAGER.initialize();
    TEXTURE_MANAGER.initialize(4);

    static auto DIFFUSE = TEXTURE_MANAGER.loadTexture("./res/terrain.bmp");
    DIFFUSE->BindTo(0);

    WORLD = World({0, 0, 0}, 4562, 16, THREAD_COUNT);
    WORLD.initializeVertexArray();

    std::thread worldUpdater[THREAD_COUNT];
    float y = C_HEIGHT;
    while (!BLOCKS[WORLD.getBlock(0, y, 0).ID].collision) {
        y--;
    }
    PLAYER = Player(&WORLD, {0.5f, y + 1.0625f, 0.5f}, {0.6f, 1.8f, 0.6f});

    for (int i = 0; i < THREAD_COUNT; i++) {
        worldUpdater[i] = std::thread(worldUpdaterThread, i);
    }

    while (true) {
        bool allFinished = false;
        for (bool thr : FIRST_UPDATE) {
            if (thr) {
                allFinished = true;
            }
        }
        if (allFinished)
            break;
        glfwSwapBuffers(WINDOW);
        glfwPollEvents();
    }

    glfwSetInputMode(WINDOW, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPos(WINDOW, WIDTH * 0.5, HEIGHT * 0.5);

    loadShader();

    while (!glfwWindowShouldClose(WINDOW)) {
        render();
        glfwPollEvents();

        glm::dvec2 mouse;
        glfwGetCursorPos(WINDOW, &mouse.x, &mouse.y);
        glfwSetCursorPos(WINDOW, static_cast<double>(WIDTH / 2), static_cast<double>(HEIGHT / 2));
        PLAYER.addAngle((mouse - glm::dvec2{WIDTH / 2, HEIGHT / 2}));
    }
    WORLD.setInactive();
    for (std::thread &thread : worldUpdater) {
        if (thread.joinable())
            thread.join();
    }

    WORLD.cleanUp();
    glfwTerminate();

    return 0;
}

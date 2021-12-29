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
static bool VSYNC = true;
static World WORLD;
static Player PLAYER;


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        SHADER_MANAGER.reloadAll();
        Shader* shader = SHADER_MANAGER.getDefault().get();
        shader->Bind();
        shader->setInt("DIFFUSE", 0);
        shader->setFloat("INV_RENDER_DIST", 1.0f / WORLD.getRenderDistance());
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    while (WORLD.isActive()) {
        WORLD.update(thrID);
    }
}


void render() {
    static double lastFPS = glfwGetTime();
    static int frames{0};
    static glm::fvec3 up(0, 1, 0);
    static glm::fmat4x4 proj = glm::perspective(
            glm::radians(65.0f),
            1.0f * WIDTH / HEIGHT,
            0.03f, 1024.0f);

    static Shader *shader = SHADER_MANAGER.getDefault().get();
    shader->Bind();

    static double lastFrame = glfwGetTime();
    double time = glfwGetTime();
    double dTime = time - lastFrame;
    lastFrame = time;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PLAYER.update(WINDOW, time, dTime);
    glm::fmat4x4 view = glm::lookAt(PLAYER.getEyePosition(), PLAYER.getEyePosition() + PLAYER.getDirection(), up);
    glm::fmat4x4 MVP = proj * view;
    shader->setMatrixFloat4("MVP", MVP);
    shader->setFloat3("PLAYER_POSITION", PLAYER.getEyePosition());
    shader->setFloat("TIME", (float)time);

    WORLD.render();

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

    glClearColor(0.8, 0.95, 1.0, 1.0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    SHADER_MANAGER.initialize();
    TEXTURE_MANAGER.initialize(4);
    static auto shader = SHADER_MANAGER.getDefault().get();
    static auto DIFFUSE = TEXTURE_MANAGER.loadTexture(
            "./res/terrain.bmp", false, false, 1
    );

    shader->Bind();
    DIFFUSE->BindTo(0);

    constexpr int thread_count = 1;
    WORLD = World({0, 0, 0}, 255, 16, thread_count);
    std::thread worldUpdater[thread_count];

    PLAYER = Player(&WORLD, {0, 224.0f, 0}, {0.6f, 1.8f, 0.6f});
    shader->setFloat("INV_RENDER_DIST", 1.0f / WORLD.getRenderDistance());

    for (int i = 0; i < thread_count; i++) {
        worldUpdater[i] = std::thread(worldUpdaterThread, i);
    }

    glfwSetInputMode(WINDOW, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPos(WINDOW, WIDTH * 0.5, HEIGHT * 0.5);

    while (!glfwWindowShouldClose(WINDOW)) {
        render();
        glfwPollEvents();

        glm::dvec2 mouse;
        glfwGetCursorPos(WINDOW, &mouse.x, &mouse.y);
        glfwSetCursorPos(WINDOW, WIDTH / 2, HEIGHT / 2);
        PLAYER.addAngle((mouse - glm::dvec2{WIDTH / 2, HEIGHT / 2}));
    }
    WORLD.setInactive();
    for (std::thread &thread : worldUpdater) {
        if (thread.joinable())
            thread.join();
    }

    glfwTerminate();

    return 0;
}

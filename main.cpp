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
#include "Player.h"
#include "ShaderManager.hpp"
#include "TextureManager.h"


static GLFWwindow *WINDOW = nullptr;
static ShaderManager SHADER_MANAGER;
static TextureManager TEXTURE_MANAGER;
static Player PLAYER;
static int WIDTH = 1280;
static int HEIGHT = 720;
static constexpr double MOUSE_SPEED = 0.1;
//static glm::fvec3 EYE(0.0f, 224.0f, 0.0f);
//static glm::fvec3 DIR(0.0f, -0.5f, 1.0f);
static glm::dvec2 ANGLE(0.0, 0.0);


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
        SHADER_MANAGER.reloadAll();
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

void worldUpdaterThread(int thrID, World *world_ptr) {
    World &world = *world_ptr;
    while (world.isActive()) {
        world.update(thrID);
    }
}


void render(World &world) {
    static double lastFPS = glfwGetTime();
    static int frames{0};
    static glm::fvec3 up(0, 1, 0);
    static glm::fmat4x4 proj = glm::perspective(
            glm::radians(65.0f),
            1.0f * WIDTH / HEIGHT,
            0.03f, 2048.0f);

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


    world.render();

    glfwSwapBuffers(WINDOW);
    frames++;

    if (time - lastFPS >= 1.0) {
        glfwSetWindowTitle(WINDOW, ("kxCraft - " + std::to_string(frames)).c_str());
        frames = 0;
        lastFPS = time;
    }

    glfwSwapInterval(1);
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

    glClearColor(0.0, 0.0, 0.0, 1.0);
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
    shader->setInt("DIFFUSE", 0);
    glActiveTexture(GL_TEXTURE0);
    DIFFUSE->Bind();

    constexpr int thread_count = 4;
    World world({0, 0, 0}, 255, 8, thread_count);
    std::thread worldUpdater[thread_count];

    PLAYER = Player(&world, {0, 224.0f, 0}, {0.6f, 1.8f, 0.6f});

    for (int i = 0; i < thread_count; i++) {
        worldUpdater[i] = std::thread(worldUpdaterThread, i, &world);
    }

    glfwSetInputMode(WINDOW, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPos(WINDOW, WIDTH * 0.5, HEIGHT * 0.5);

    while (!glfwWindowShouldClose(WINDOW)) {
        render(world);
        glfwPollEvents();

        glm::dvec2 mouse;
        glfwGetCursorPos(WINDOW, &mouse.x, &mouse.y);
        glfwSetCursorPos(WINDOW, WIDTH*0.5, HEIGHT*0.5);
        ANGLE += (mouse - glm::dvec2{WIDTH*0.5, HEIGHT*0.5}) * MOUSE_SPEED;
        ANGLE.y = glm::clamp(ANGLE.y, -90.0, 85.0);
    }
    world.setInactive();
    for (std::thread &thread : worldUpdater) {
        if (thread.joinable())
            thread.join();
    }

    glfwTerminate();

    return 0;
}

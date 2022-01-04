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
static int WIDTH = 2560;
static int HEIGHT = 1080;
static constexpr bool VSYNC = true;
static constexpr int THREAD_COUNT = 1;
static bool FIRST_UPDATE[THREAD_COUNT] = { false };
static World *WORLD = nullptr;
static Player *PLAYER = nullptr;



void loadShader() {
    Shader& shader = SHADER_MANAGER.getShader("./res/shader/terrain");
    shader.Bind();
    shader.setInt("DIFFUSE", 0);
    shader.setFloat("INV_RENDER_DIST", 1.0f / WORLD->getRenderDistance());
    shader.setBool("DISTANCE_CULLING", true);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    PLAYER->addAngle(xpos - WIDTH * 0.5, ypos - HEIGHT * 0.5);
    glfwSetCursorPos(WINDOW, 0.5 * WIDTH, 0.5 * HEIGHT);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        SHADER_MANAGER.reloadAll();
        loadShader();
    }
    if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
        WORLD->reloadCurrentChunk();
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (PLAYER != nullptr) {
        PLAYER->scrollItems(-yoffset);
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

    WINDOW = glfwCreateWindow(WIDTH, HEIGHT, "kxCraft", glfwGetPrimaryMonitor(), nullptr);
    if (!WINDOW) {
        std::cerr << "Couldn't create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwSetKeyCallback(WINDOW, key_callback);
    glfwSetWindowSizeCallback(WINDOW, window_size_callback);
    glfwSetScrollCallback(WINDOW, scroll_callback);
    glfwSetCursorPosCallback(WINDOW, cursor_position_callback);
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
    std::cout << "Updater Thread " << thrID << std::endl;
    while (WORLD->isActive()) {
        WORLD->update(thrID);
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

    PLAYER->update(WINDOW, time, dTime);

    glm::fvec3 playerEyePosition = PLAYER->getEyePosition();

    glm::fmat4x4 view = glm::lookAt(playerEyePosition, playerEyePosition + PLAYER->getDirection(), up);
    glm::fmat4x4 MVP = proj * view;

    terrainShader.Bind();
    terrainShader.setMatrixFloat4("MVP", MVP);
    terrainShader.setFloat3("PLAYER_POSITION", playerEyePosition);
    terrainShader.setFloat("TIME", (float)time);
    terrainShader.setBool("HUD", false);

    WORLD->render(glfwGetKey(WINDOW, GLFW_KEY_LEFT_ALT));
    terrainShader.setBool("HUD", true);
    PLAYER->render();

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

    SHADER_MANAGER.initialize();
    TEXTURE_MANAGER.initialize(4);

    static auto DIFFUSE = TEXTURE_MANAGER.loadTexture("./res/terrain.bmp");
    DIFFUSE->BindTo(0);

    WORLD = new World({0, 0, 0}, 4562, 16, THREAD_COUNT);
    WORLD->initializeVertexArray();

    float y = C_HEIGHT-1.0f;
    while (WORLD->getBlock(0, y, 0).ID == AIR)
        y--;
    PLAYER = new Player(WORLD, {0.5f, y + 1.05f, 0.5f}, {0.6f, 1.8f, 0.6f});

    std::thread worldUpdater[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        worldUpdater[i] = std::thread(worldUpdaterThread, i);
    }

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT);
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
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(WINDOW, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glClearColor(0.75, 0.9, 1.0, 1.0);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    loadShader();

    while (!glfwWindowShouldClose(WINDOW)) {
        render();
        glfwPollEvents();
    }

    std::cout << "Exit game" << std::endl;
    WORLD->setInactive();
    for (std::thread &thread : worldUpdater) {
        if (thread.joinable())
            thread.join();
    }

    delete PLAYER;
    delete WORLD;
    glfwTerminate();

    return 0;
}

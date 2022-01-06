//
// Created by jana on 28.12.21.
//

#include "Player.hpp"
#include <glm/ext.hpp>
#include "Block.hpp"
#include <iostream>

static constexpr double MOUSE_SPEED = 0.05;
static constexpr float MOVEMENT_SPEED = 2.0f;
static constexpr float RUN_MULTI = 3.0f;

Player::Player(World *world, const glm::fvec3 &position, const glm::fvec3 &bbox)
        : position(position), bbox(bbox), world(world) {

}

void Player::render() {
    glDisable(GL_DEPTH_TEST);
    auto eye = getEyePosition();
    hud.begin();
    if (hasHeadingBlock) {
        hud.addBlock(headingBlock + 0.5f, BLOCK_ID::SELECTION, 255);
    }

    static const int count = sizeof(BUILDABLE) / sizeof(BUILDABLE[0]);
    for (int i = 0; i < count; i++) {
        if (i != blockID) {
            float interp = 1/(1 + pow(i-blockID, 4));
            hud.addBlockScreenSpace(TBN, eye,
                                    {-count + i * 2.0f + 1.0f, -19.0f + interp, 24.0f - 8.0f * interp},
                                    BUILDABLE[i], 50);
        }
    }
    hud.addBlockScreenSpace(TBN, eye, {-count + blockID * 2.0f + 1.0f, -18.0f, 16.0f}, BUILDABLE[blockID], 120);
    hud.end();
    glEnable(GL_DEPTH_TEST);
}

void Player::update(GLFWwindow *window, const double &time, const double &dTime) {
    handleKeys(window, time, dTime);

    auto rotX = glm::rotate(glm::fmat4x4(1), glm::radians( (float)cameraAngle.y), glm::fvec3(1, 0, 0));
    auto rotY = glm::rotate(glm::fmat4x4(1), glm::radians( (float)cameraAngle.x), glm::fvec3(0, -1, 0));
    TBN = rotY * rotX;
    direction = TBN[2];

    if (glm::dot(velocity, velocity) > 0) {
        float dx = velocity.x * static_cast<float>(dTime) * MOVEMENT_SPEED;
        float dy = velocity.y * static_cast<float>(dTime);
        float dz = velocity.z * static_cast<float>(dTime) * MOVEMENT_SPEED;

        if (!collide(position.x + dx, position.y, position.z))
            position.x += dx;

        if (!collide(position.x, position.y + dy, position.z)) {
            if (velocity.y < 0)
                canJump = false;
            position.y += dy;
        }
        else {
            if (velocity.y < 0)
                canJump = true;
            velocity.y = 0;
        }

        if (!collide(position.x, position.y, position.z + dz))
            position.z += dz;

    }
    velocity.y -= static_cast<float>(dTime)*2*9.81f;

    world->setPlayer(position, direction);

    hasHeadingBlock = false;
    for (int d = 3; d < 55; d++) {
        headingBlock = glm::floor(getEyePosition() + direction * (0.1f * d));
        const Block &attr = world->getBlockAttributes(headingBlock.x, headingBlock.y, headingBlock.z);
        if (attr.collision || attr.isPlant || (!attr.translucent && !attr.transparent)) {
            buildBlock = glm::floor(getEyePosition() + direction * (0.1f * (d - 1)));
            hasHeadingBlock = true;
            break;
        }
    }

    bool attack = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (hasHeadingBlock && attack && (time - lastAttack > 0.2)) {
        lastAttack = time;
        world->setBlock(headingBlock.x, headingBlock.y, headingBlock.z, AIR, true);
    }
    else if (!attack)
        lastAttack = 0.0;

    bool build = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    if (hasHeadingBlock && build && (time - lastBuild > 0.2)) {
        lastBuild = time;
        world->setBlock(buildBlock.x, buildBlock.y, buildBlock.z, BUILDABLE[blockID], true);
    }
    else if (!build)
        lastBuild = 0.0;
}

void Player::addAngle(double ax, double ay) {
    cameraAngle += glm::dvec2(ax, ay) * MOUSE_SPEED;
    cameraAngle.y = glm::clamp(cameraAngle.y, -89.0, 89.0);
    while (cameraAngle.x > 360.0)
        cameraAngle.x -= 360.0;
    while (cameraAngle.x < 0.0)
        cameraAngle.x += 360.0;
}

void Player::handleKeys(GLFWwindow *window, const double &time, const double &dTime) {
    velocity.x = 0;
    velocity.z = 0;
    double angle = glm::radians(cameraAngle.x);

    float speed = 1.0f + RUN_MULTI * static_cast<float>(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT));
    walking = -1.0f;

    if (glfwGetKey(window, GLFW_KEY_W)) {
        walking = time;
        velocity += glm::fvec3(-glm::sin(angle), 0, glm::cos(angle)) * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_A)) {
        walking = time;
        velocity += glm::fvec3(glm::cos(angle), 0, glm::sin(angle)) * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
        walking = time;
        velocity -= glm::fvec3(glm::cos(angle), 0, glm::sin(angle)) * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
        walking = time;
        velocity += glm::fvec3(glm::sin(angle), 0, -glm::cos(angle)) * speed;
    }
    if (canJump && glfwGetKey(window, GLFW_KEY_SPACE)) {
        canJump = false;
        velocity.y = 6.85f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
        velocity.y -= 1;
    }

}

bool Player::collide(float x, float y, float z) const {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 2; k++) {
                const st_block &block = world->getBlock(
                        x + (-0.5f + (float) i) * bbox.x,
                        y + (0.5f * (float) j) * bbox.y,
                        z + (-0.5f + (float) k) * bbox.z
                );

                if (BLOCKS[block.ID].collision) {
                    return true;
                }
            }
        }
    }

    return false;
}



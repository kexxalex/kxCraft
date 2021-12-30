//
// Created by jana on 28.12.21.
//

#include "Player.hpp"
#include <glm/ext.hpp>
#include "Block.hpp"
#include <iostream>

static constexpr double MOUSE_SPEED = 0.1;
static constexpr float MOVEMENT_SPEED = 3.5f;

Player::Player(World *world, const glm::fvec3 &position, const glm::fvec3 &bbox)
        : position(position), bbox(bbox), world(world) {

}

void Player::update(GLFWwindow *window, const double &time, const double &dTime) {

    handleKeys(window, time, dTime);

    auto rotX = glm::rotate(glm::mat4x4(1), glm::radians((float) cameraAngle.y), glm::fvec3(1, 0, 0));
    auto rotY = glm::rotate(glm::mat4x4(1), glm::radians((float) cameraAngle.x), glm::fvec3(0, -1, 0));
    direction = (rotY * rotX * glm::fvec4(0, 0, 1, 0));

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
        headingBlock = getEyePosition() + direction * (0.1f * d);
        if (world->getBlock(headingBlock.x, headingBlock.y, headingBlock.z).ID != AIR) {
            buildBlock = getEyePosition() + direction * (0.1f * (d-1));
            hasHeadingBlock = true;
            break;
        }
    }

    bool attack = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (hasHeadingBlock && attack && (time - lastAttack > 0.2)) {
        lastAttack = time;
        world->setBlock(headingBlock.x, headingBlock.y, headingBlock.z, AIR, true);
    }
    else if (!attack || !hasHeadingBlock)
        lastAttack = 0.0;

    bool build = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    if (hasHeadingBlock && build && (time - lastBuild > 0.2)) {
        lastBuild = time;
        world->setBlock(buildBlock.x, buildBlock.y, buildBlock.z, STONE, true);
    }
    else if (!build || !hasHeadingBlock)
        lastBuild = 0.0;
}

void Player::addAngle(const glm::dvec2 &angle) {
    cameraAngle += angle * MOUSE_SPEED;
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

    if (glfwGetKey(window, GLFW_KEY_W)) {
        velocity += glm::fvec3(-glm::sin(angle), 0, glm::cos(angle));
    }
    if (glfwGetKey(window, GLFW_KEY_A)) {
        velocity += glm::fvec3(glm::cos(angle), 0, glm::sin(angle));
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
        velocity -= glm::fvec3(glm::cos(angle), 0, glm::sin(angle));
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
        velocity += glm::fvec3(glm::sin(angle), 0, -glm::cos(angle));
    }
    if (canJump && glfwGetKey(window, GLFW_KEY_SPACE)) {
        canJump = false;
        velocity.y = 6.8f;
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



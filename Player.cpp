//
// Created by jana on 28.12.21.
//

#include "Player.hpp"
#include <glm/ext.hpp>
#include "Block.hpp"
#include <iostream>

static constexpr double MOUSE_SPEED = 0.1;

Player::Player(World *world, const glm::fvec3 &position, const glm::fvec3 &bbox)
        : position(position), bbox(bbox), world(world) {

}

void Player::update(GLFWwindow *window, const double &time, const double &dTime) {

    handleKeys(window, dTime);

    auto rotX = glm::rotate(glm::mat4x4(1), glm::radians((float) cameraAngle.y), glm::fvec3(1, 0, 0));
    auto rotY = glm::rotate(glm::mat4x4(1), glm::radians((float) cameraAngle.x), glm::fvec3(0, -1, 0));
    direction = (rotY * rotX * glm::fvec4(0, 0, 1, 0));

    if (glm::dot(velocity, velocity) > 0) {
        glm::fvec3 delta = glm::normalize(velocity) * (static_cast<float>(dTime) * C_EXTEND);

        std::cout << position.x << " " << position.y << " " << position.z << " -- "
                  << collide(position.x + delta.x, position.y, position.z) << std::endl;

        if (!collide(position.x + delta.x, position.y, position.z))
            position.x += delta.x;

        if (!collide(position.x, position.y + delta.y, position.z))
            position.y += delta.y;

        if (!collide(position.x, position.y, position.z + delta.z))
            position.z += delta.z;

    }

    world->setPlayer(position, direction);
}

void Player::addAngle(const glm::dvec2 &angle) {
    cameraAngle += angle * MOUSE_SPEED;
    cameraAngle.y = glm::clamp(cameraAngle.y, -90.0, 85.0);
}

void Player::handleKeys(GLFWwindow *window, const double &dTime) {
    velocity = {0, 0, 0};
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
    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
        position.y += (float) dTime * C_EXTEND;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
        position.y -= (float) dTime * C_EXTEND;
    }

}

bool Player::collide(float x, float y, float z) const {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 2; k++) {
                std::cout << x << " " << y << " " << z << std::endl;
                const auto &block = world->getBlock(
                        x + (-0.5f + (float) i) * bbox.x,
                        y + (0.5f * (float) j) * bbox.y,
                        z + (-0.5f + (float) k) * bbox.z
                );

                if (block.ID != AIR) {
                    return true;
                }
            }
        }

    }

    return false;
}



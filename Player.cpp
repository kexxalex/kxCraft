//
// Created by jana on 28.12.21.
//

#include "Player.h"
#include <glm/ext.hpp>


Player::Player(World *world, const glm::fvec3 &position, const glm::fvec3 &bbox)
        : position(position), bbox(bbox), world(world) {

}

void Player::update(GLFWwindow *window, const double &time, const double &dTime) {
    velocity = {0, 0, 0};
    if (glfwGetKey(window, GLFW_KEY_W)) {
        velocity += glm::fvec3(glm::sin(cameraAngle.x), 0, glm::cos(cameraAngle.y));
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
        velocity += glm::fvec3(-glm::cos(cameraAngle.x), 0, glm::sin(cameraAngle.y));
    }
    if (glfwGetKey(window, GLFW_KEY_A)) {
        velocity += glm::fvec3(glm::cos(cameraAngle.x), 0, -glm::sin(cameraAngle.y));
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
        velocity -= glm::fvec3(glm::sin(cameraAngle.x), 0, glm::cos(cameraAngle.y));
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
        position.y += (float) dTime * C_EXTEND;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
        position.y -= (float) dTime * C_EXTEND;
    }

    auto rotX = glm::rotate(glm::mat4x4(1), glm::radians((float) cameraAngle.y), glm::fvec3(1, 0, 0));
    auto rotY = glm::rotate(glm::mat4x4(1), glm::radians((float) cameraAngle.x), glm::fvec3(0, -1, 0));
    direction = (rotY * rotX * glm::fvec4(0, 0, 1, 0));

    if (glm::dot(velocity, velocity) > 0)
        position += glm::normalize(velocity) * (static_cast<float>(dTime) * C_EXTEND);

    world->setPlayer(position, direction);
}



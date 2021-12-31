//
// Created by jana on 28.12.21.
//
#pragma once

#include <glm/glm.hpp>
#include "World.hpp"
#include <GLFW/glfw3.h>


class Player {
public:
    Player() = default;

    Player(World *world, const glm::fvec3 &position, const glm::fvec3 &bbox);

    void addAngle(const glm::dvec2 &angle);

    void update(GLFWwindow *window, const double &time, const double &dTime);

    [[nodiscard]] inline const glm::fvec3 &getPosition() const noexcept { return position; }

    [[nodiscard]] inline const glm::fvec3 &getDirection() const noexcept { return direction; }

    [[nodiscard]] inline glm::fvec3 getEyePosition() const noexcept {
        return position + glm::fvec3(0, bbox.y - 0.1f, 0);
    }

private:
    glm::fvec3 position{0, 0, 0};
    glm::fvec3 direction{0, 0, 1};
    glm::fvec3 velocity{0, 0, 0};
    glm::dvec2 cameraAngle{0, 0};

    glm::fvec3 bbox{0.8f, 1.8f, 0.8f};

    World *world{nullptr};

    bool canJump{false};
    bool isFlying{false};

    bool hasHeadingBlock{ false };
    glm::fvec3 headingBlock{ 0, 0, 0};
    glm::fvec3 buildBlock{ 0, 0, 0};

    double lastAttack{0.0};
    double lastBuild{0.0};

    void handleKeys(GLFWwindow *window, const double &time, const double &dTime);

    [[nodiscard]] bool collide(float x, float y, float z) const;

};

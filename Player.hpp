//
// Created by jana on 28.12.21.
//
#pragma once

#include <glm/glm.hpp>
#include "World.hpp"
#include <GLFW/glfw3.h>
#include "HUD.hpp"



class Player {
public:
    Player() = default;

    Player(World *world, const glm::fvec3 &position, const glm::fvec3 &bbox);

    void addAngle(double ax, double ay);
    inline void scrollItems(int dir) {
        blockID += dir;
        blockID = MOD(blockID,  sizeof(BUILDABLE));
    }

    void update(GLFWwindow *window, const double time, const double dTime);
    void render();

    [[nodiscard]] constexpr const glm::fvec3 &getPosition() const noexcept { return position; }

    [[nodiscard]] constexpr const glm::fvec3 &getDirection() const noexcept { return direction; }

    [[nodiscard]] constexpr glm::fvec3 getEyePosition() const noexcept {
        return position + glm::fvec3(-(walking > 0)*glm::sin(walking * PI2) * 0.025 * direction.z,
                                     bbox.y - 0.1 + (walking > 0)*glm::abs(glm::sin(walking * PI2))* 0.05,
                                     (walking > 0)*glm::sin(walking * PI2)* 0.025 * direction.x);
    }

private:
    glm::fvec3 position{0, 0, 0};
    glm::fvec3 direction{0, 0, 1};
    glm::fvec3 velocity{ 0, 0, 0};
    glm::dvec2 cameraAngle{90, 0};

    glm::fvec3 bbox{0.8f, 1.8f, 0.8f};

    HUD hud;
    World *world{nullptr};

    bool canJump{false};
    bool isFlying{false};
    bool isCrouching{ false };

    bool hasHeadingBlock{ false };
    glm::fvec3 headingBlock{ 0, 0, 0};
    glm::fvec3 buildBlock{ 0, 0, 0};
    int blockID{ 2 };

    glm::fmat3x3 TBN;

    double lastAttack{0.0};
    double lastBuild{0.0};
    double walking{-1.0};

    void handleKeys(GLFWwindow *window, const double &time, const double &dTime);

    [[nodiscard]] bool collide(float x, float y, float z) const;

};

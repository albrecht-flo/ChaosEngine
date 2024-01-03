#pragma once

#include <glm/glm.hpp>

namespace GLMCustomExtension {

    inline glm::mat2x2 rotation2D(float theta) {
        theta = glm::radians(theta);
        return glm::mat2x2{
                cos(theta), -sin(theta),
                sin(theta), cos(theta)
        };
    }

}

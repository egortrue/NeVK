#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace nevk
{
struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos && normal == other.normal && uv == other.uv;
    }
};
} // namespace nevk

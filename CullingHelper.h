#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <vector>
#include <array>

// from https://bruop.github.io/improved_frustum_culling/

struct CullingFrustum
{
    float near_right;
    float near_top;
    float near_plane;
    float far_plane;
};

struct AABB
{
    glm::vec3 min;
    glm::vec3 max;
};

struct OBB
{
    glm::vec3 center = {};
    glm::vec3 extents = {};
    glm::vec3 axes[3] = {};
};

AABB createAABB(const std::vector<char> &vertices, uint32_t stride, uint32_t posOffset, uint32_t normalOffset);

bool test_using_separating_axis_theorem(const CullingFrustum &frustum, const glm::mat4 &vs_transform, const AABB &aabb);
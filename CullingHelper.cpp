#include "CullingHelper.h"

AABB createAABB(const std::vector<char> &vertices, uint32_t stride, uint32_t posOffset, uint32_t normalOffset)
{
    if (normalOffset - posOffset != 12)
        throw std::runtime_error("failed to parse vertex position, not 4-byte data!");

    float min_x = FLT_MAX, min_y = FLT_MAX, min_z = FLT_MAX;
    float max_x = FLT_MIN, max_y = FLT_MIN, max_z = FLT_MIN;
    for (size_t i = 0; i * stride < vertices.size(); ++i)
    {
        float pos_x = std::bit_cast<float>(std::array<char, 4>{vertices[i * stride], vertices[i * stride + 1], vertices[i * stride + 2], vertices[i * stride + 3]});
        float pos_y = std::bit_cast<float>(std::array<char, 4>{vertices[i * stride + 4], vertices[i * stride + 5], vertices[i * stride + 6], vertices[i * stride + 7]});
        float pos_z = std::bit_cast<float>(std::array<char, 4>{vertices[i * stride + 8], vertices[i * stride + 9], vertices[i * stride + 10], vertices[i * stride + 11]});

        if (pos_x < min_x)
        {
            min_x = pos_x;
        }
        else if (pos_x > max_x)
        {
            max_x = pos_x;
        }
        if (pos_y < min_y)
        {
            min_y = pos_y;
        }
        else if (pos_y > max_y)
        {
            max_y = pos_y;
        }
        if (pos_z < min_z)
        {
            min_z = pos_z;
        }
        else if (pos_z > max_z)
        {
            max_z = pos_z;
        }
    }

    AABB aabb{.min = glm::vec3(min_x, min_y, min_z), .max = glm::vec3(max_x, max_y, max_z)};
    return aabb;
}

bool test_using_separating_axis_theorem(const CullingFrustum &frustum, const glm::mat4 &vs_transform, const AABB &aabb)
{
    // Near, far
    float z_near = frustum.near_plane;
    float z_far = frustum.far_plane;
    // half width, half height
    float x_near = frustum.near_right;
    float y_near = frustum.near_top;

    // So first thing we need to do is obtain the normal directions of our OBB by transforming 4 of our AABB vertices
    glm::vec3 corners[] = {
        {aabb.min.x, aabb.min.y, aabb.min.z},
        {aabb.max.x, aabb.min.y, aabb.min.z},
        {aabb.min.x, aabb.max.y, aabb.min.z},
        {aabb.min.x, aabb.min.y, aabb.max.z},
    };

    // Transform corners
    // This only translates to our OBB if our transform is affine
    for (size_t corner_idx = 0; corner_idx < sizeof(corners) / sizeof(corners[0]); corner_idx++)
    {
        corners[corner_idx] = glm::vec3(vs_transform * glm::vec4(corners[corner_idx], 1.0f));
    }

    OBB obb = {
        .axes = {
            corners[1] - corners[0],
            corners[2] - corners[0],
            corners[3] - corners[0]},
    };
    obb.center = corners[0] + 0.5f * (obb.axes[0] + obb.axes[1] + obb.axes[2]);
    obb.extents = glm::vec3{length(obb.axes[0]), length(obb.axes[1]), length(obb.axes[2])};
    obb.axes[0] = obb.axes[0] / obb.extents.x;
    obb.axes[1] = obb.axes[1] / obb.extents.y;
    obb.axes[2] = obb.axes[2] / obb.extents.z;
    obb.extents *= 0.5f;

    {
        glm::vec3 M = {0, 0, 1};
        float MoX = 0.0f;
        float MoY = 0.0f;
        float MoZ = 1.0f;

        // Projected center of our OBB
        float MoC = obb.center.z;
        // Projected size of OBB
        float radius = 0.0f;
        for (size_t i = 0; i < 3; i++)
        {
            // dot(M, axes[i]) == axes[i].z;
            radius += fabsf(obb.axes[i].z) * obb.extents[i];
        }
        float obb_min = MoC - radius;
        float obb_max = MoC + radius;

        float tau_0 = z_far; // Since z is negative, far is smaller than near
        float tau_1 = z_near;

        if (obb_min > tau_1 || obb_max < tau_0)
        {
            return false;
        }
    }

    {
        const glm::vec3 M[] = {
            {z_near, 0.0f, x_near},  // Left Plane
            {-z_near, 0.0f, x_near}, // Right plane
            {0.0, -z_near, y_near},  // Top plane
            {0.0, z_near, y_near},   // Bottom plane
        };
        for (size_t m = 0; m < sizeof(M) / sizeof(M[0]); m++)
        {
            float MoX = fabsf(M[m].x);
            float MoY = fabsf(M[m].y);
            float MoZ = M[m].z;
            float MoC = dot(M[m], obb.center);

            float obb_radius = 0.0f;
            for (size_t i = 0; i < 3; i++)
            {
                obb_radius += fabsf(dot(M[m], obb.axes[i])) * obb.extents[i];
            }
            float obb_min = MoC - obb_radius;
            float obb_max = MoC + obb_radius;

            float p = x_near * MoX + y_near * MoY;

            float tau_0 = z_near * MoZ - p;
            float tau_1 = z_near * MoZ + p;

            if (tau_0 < 0.0f)
            {
                tau_0 *= z_far / z_near;
            }
            if (tau_1 > 0.0f)
            {
                tau_1 *= z_far / z_near;
            }

            if (obb_min > tau_1 || obb_max < tau_0)
            {
                return false;
            }
        }
    }

    // OBB Axes
    {
        for (size_t m = 0; m < sizeof(obb.axes) / sizeof(obb.axes[0]); m++)
        {
            const glm::vec3 &M = obb.axes[m];
            float MoX = fabsf(M.x);
            float MoY = fabsf(M.y);
            float MoZ = M.z;
            float MoC = dot(M, obb.center);

            float obb_radius = obb.extents[m];

            float obb_min = MoC - obb_radius;
            float obb_max = MoC + obb_radius;

            // Frustum projection
            float p = x_near * MoX + y_near * MoY;
            float tau_0 = z_near * MoZ - p;
            float tau_1 = z_near * MoZ + p;
            if (tau_0 < 0.0f)
            {
                tau_0 *= z_far / z_near;
            }
            if (tau_1 > 0.0f)
            {
                tau_1 *= z_far / z_near;
            }

            if (obb_min > tau_1 || obb_max < tau_0)
            {
                return false;
            }
        }
    }

    // Now let's perform each of the cross products between the edges
    // First R x A_i
    {
        for (size_t m = 0; m < sizeof(obb.axes) / sizeof(obb.axes[0]); m++)
        {
            const glm::vec3 M = {0.0f, -obb.axes[m].z, obb.axes[m].y};
            float MoX = 0.0f;
            float MoY = fabsf(M.y);
            float MoZ = M.z;
            float MoC = M.y * obb.center.y + M.z * obb.center.z;

            float obb_radius = 0.0f;
            for (size_t i = 0; i < 3; i++)
            {
                obb_radius += fabsf(dot(M, obb.axes[i])) * obb.extents[i];
            }

            float obb_min = MoC - obb_radius;
            float obb_max = MoC + obb_radius;

            // Frustum projection
            float p = x_near * MoX + y_near * MoY;
            float tau_0 = z_near * MoZ - p;
            float tau_1 = z_near * MoZ + p;
            if (tau_0 < 0.0f)
            {
                tau_0 *= z_far / z_near;
            }
            if (tau_1 > 0.0f)
            {
                tau_1 *= z_far / z_near;
            }

            if (obb_min > tau_1 || obb_max < tau_0)
            {
                return false;
            }
        }
    }

    // U x A_i
    {
        for (size_t m = 0; m < sizeof(obb.axes) / sizeof(obb.axes[0]); m++)
        {
            const glm::vec3 M = {obb.axes[m].z, 0.0f, -obb.axes[m].x};
            float MoX = fabsf(M.x);
            float MoY = 0.0f;
            float MoZ = M.z;
            float MoC = M.x * obb.center.x + M.z * obb.center.z;

            float obb_radius = 0.0f;
            for (size_t i = 0; i < 3; i++)
            {
                obb_radius += fabsf(dot(M, obb.axes[i])) * obb.extents[i];
            }

            float obb_min = MoC - obb_radius;
            float obb_max = MoC + obb_radius;

            // Frustum projection
            float p = x_near * MoX + y_near * MoY;
            float tau_0 = z_near * MoZ - p;
            float tau_1 = z_near * MoZ + p;
            if (tau_0 < 0.0f)
            {
                tau_0 *= z_far / z_near;
            }
            if (tau_1 > 0.0f)
            {
                tau_1 *= z_far / z_near;
            }

            if (obb_min > tau_1 || obb_max < tau_0)
            {
                return false;
            }
        }
    }

    // Frustum Edges X Ai
    {
        for (size_t obb_edge_idx = 0; obb_edge_idx < sizeof(obb.axes) / sizeof(obb.axes[0]); obb_edge_idx++)
        {
            const glm::vec3 M[] = {
                cross({-x_near, 0.0f, z_near}, obb.axes[obb_edge_idx]), // Left Plane
                cross({x_near, 0.0f, z_near}, obb.axes[obb_edge_idx]),  // Right plane
                cross({0.0f, y_near, z_near}, obb.axes[obb_edge_idx]),  // Top plane
                cross({0.0, -y_near, z_near}, obb.axes[obb_edge_idx])   // Bottom plane
            };

            for (size_t m = 0; m < sizeof(M) / sizeof(M[0]); m++)
            {
                float MoX = fabsf(M[m].x);
                float MoY = fabsf(M[m].y);
                float MoZ = M[m].z;

                constexpr float epsilon = 1e-4;
                if (MoX < epsilon && MoY < epsilon && fabsf(MoZ) < epsilon)
                    continue;

                float MoC = dot(M[m], obb.center);

                float obb_radius = 0.0f;
                for (size_t i = 0; i < 3; i++)
                {
                    obb_radius += fabsf(dot(M[m], obb.axes[i])) * obb.extents[i];
                }

                float obb_min = MoC - obb_radius;
                float obb_max = MoC + obb_radius;

                // Frustum projection
                float p = x_near * MoX + y_near * MoY;
                float tau_0 = z_near * MoZ - p;
                float tau_1 = z_near * MoZ + p;
                if (tau_0 < 0.0f)
                {
                    tau_0 *= z_far / z_near;
                }
                if (tau_1 > 0.0f)
                {
                    tau_1 *= z_far / z_near;
                }

                if (obb_min > tau_1 || obb_max < tau_0)
                {
                    return false;
                }
            }
        }
    }

    // No intersections detected
    return true;
};
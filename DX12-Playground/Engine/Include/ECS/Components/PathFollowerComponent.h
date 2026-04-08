#pragma once
#include <memory>
#include <Math/Path.h>

// PathFollowerComponent requires TransformComponent on the same entity.
// The PathFollowerSystem enforces this through its signature: only entities
// with both TransformComponent AND PathFollowerComponent are processed.
struct PathFollowerComponent
{
    std::shared_ptr<Math::Path> path;           // null = no path assigned

    float speed        = 1.f;   // world-space units per second
    float t            = 0.f;   // current parameter ∈ [0, 1]
    bool  loop         = true;  // wrap t at 1.0 instead of clamping
    bool  playing      = false; // advance t each frame
    bool  orientToPath = false; // align yaw (rotation.y) to tangent direction

    // Cached arc length; set to -1.f to force recompute on next Update().
    // Reset this whenever 'path' is replaced.
    float arcLength    = -1.f;
};

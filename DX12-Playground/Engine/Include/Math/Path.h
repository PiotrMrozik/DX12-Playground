#pragma once

#include <DirectXMath.h>
#include <cstdint>

namespace Math {

// Abstract base class for parametric geometric paths.
// All paths are evaluated over t ∈ [0, 1].
class Path
{
public:
    virtual ~Path() = default;

    // Returns the point on the path at parameter t ∈ [0, 1].
    virtual DirectX::XMFLOAT3 Evaluate(float t) const = 0;

    // Returns the unit tangent direction at parameter t.
    // Default implementation uses central finite differences;
    // subclasses may override with an analytical version.
    virtual DirectX::XMFLOAT3 Tangent(float t) const;

    // Approximates total arc length by sampling the path uniformly.
    float ArcLength(uint32_t samples = 128) const;
};

} // namespace Math

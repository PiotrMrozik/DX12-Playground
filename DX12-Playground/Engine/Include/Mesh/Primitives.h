#pragma once

#include <cstdint>

#include <Mesh/MeshData.h>

/// Pure-CPU factory functions that generate primitive geometry.
/// Returns MeshData ready to be uploaded via MeshRegistry::Add().
namespace Primitives
{
MeshData CreateCube(float size = 1.0f);
MeshData CreateSphere(float radius = 0.5f, uint32_t slices = 32, uint32_t stacks = 32);
MeshData CreatePlane(float width = 1.0f, float depth = 1.0f);
MeshData CreateCylinder(float radius = 0.5f, float height = 1.0f, uint32_t slices = 16);
MeshData CreateTorus(float majorRadius = 0.5f, float minorRadius = 0.2f, uint32_t majorSegments = 64,
                     uint32_t minorSegments = 16);
} // namespace Primitives

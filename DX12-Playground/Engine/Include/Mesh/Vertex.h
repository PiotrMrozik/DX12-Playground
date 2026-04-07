#pragma once
#include <DirectXMath.h>

using namespace DirectX;

struct Vertex
{
    XMFLOAT3 Position;
};

struct VertexColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

struct VertexNormal
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
};

struct VertexNormalColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT3 Color;
};

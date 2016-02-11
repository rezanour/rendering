#pragma once

class Texture2D;

// View information for rendering
struct RenderView
{
    XMFLOAT4X4 WorldToView;
    XMFLOAT4X4 ViewToProjection;
    XMFLOAT3 EyePosition;
    float NearClipDistance;
    float FarClipDistance;
};

struct RenderTarget
{
    std::shared_ptr<Texture2D> Texture;
    D3D11_VIEWPORT Viewport;
};

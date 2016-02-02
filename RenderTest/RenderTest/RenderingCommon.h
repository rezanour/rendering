#pragma once

// View information for rendering
struct RenderView
{
    XMFLOAT4X4 WorldToView;
    XMFLOAT4X4 ViewToProjection;
};

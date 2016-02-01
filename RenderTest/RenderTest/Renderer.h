#pragma once

// Light-prepass forward renderer
class Renderer
{
public:
    Renderer(ID3D11Device* device);
    virtual ~Renderer();

private:
    Renderer(const Renderer&) = delete;
    Renderer& operator= (const Renderer&) = delete;

private:
    Microsoft::WRL::ComPtr<ID3D11Device> Device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context;
};

#include "Precomp.h"
#include "Renderer.h"

using namespace Microsoft::WRL;

Renderer::Renderer(ID3D11Device* device)
    : Device(device)
{
}

Renderer::~Renderer()
{
}


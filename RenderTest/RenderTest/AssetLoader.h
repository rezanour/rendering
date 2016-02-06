#pragma once

class RenderVisual;

class AssetLoader : NonCopyable
{
public:
    AssetLoader(const ComPtr<ID3D11Device>& device, const std::wstring& assetRoot);
    virtual ~AssetLoader();

    HRESULT LoadModel(const std::wstring& relativePath, std::vector<std::shared_ptr<RenderVisual>>* visuals);

private:
    ComPtr<ID3D11Device> Device;
    std::wstring AssetRoot;
};

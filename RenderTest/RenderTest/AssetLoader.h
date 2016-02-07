#pragma once

class RenderVisual;
class Texture2D;

class AssetLoader : NonCopyable
{
public:
    AssetLoader(const ComPtr<ID3D11Device>& device, const std::wstring& assetRoot);
    virtual ~AssetLoader();

    HRESULT LoadModel(const std::wstring& relativePath, std::vector<std::shared_ptr<RenderVisual>>* visuals);
    HRESULT LoadTexture(const std::wstring& relativePath, std::shared_ptr<Texture2D>* texture);

private:
    ComPtr<ID3D11Device> Device;
    std::wstring AssetRoot;
    std::map<std::wstring, std::shared_ptr<Texture2D>> CachedTextures;
};

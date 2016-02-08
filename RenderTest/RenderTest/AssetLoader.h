#pragma once

class GraphicsDevice;
class RenderVisual;
class Texture2D;

class AssetLoader : NonCopyable
{
public:
    AssetLoader(const std::shared_ptr<GraphicsDevice>& graphics, const std::wstring& assetRoot);
    virtual ~AssetLoader();

    HRESULT LoadModel(const std::wstring& relativePath, std::vector<std::shared_ptr<RenderVisual>>* visuals);
    HRESULT LoadTexture(const std::wstring& relativePath, std::shared_ptr<Texture2D>* texture);

private:
    std::shared_ptr<GraphicsDevice> Graphics;
    std::wstring AssetRoot;
    std::map<std::wstring, std::shared_ptr<Texture2D>> CachedTextures;
};

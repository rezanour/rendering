#pragma once

class RenderVisual;

class AssetLoader : NonCopyable
{
public:
    AssetLoader(const std::wstring& assetRoot);
    virtual ~AssetLoader();

    HRESULT LoadModel(const std::wstring& relativePath, std::shared_ptr<RenderVisual>* visual);

private:
    std::wstring AssetRoot;
};

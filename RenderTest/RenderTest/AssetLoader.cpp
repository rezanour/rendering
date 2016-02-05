#include "Precomp.h"
#include "AssetLoader.h"
#include "VertexFormats.h"
#include "VertexBuffer.h"
#include "RenderVisual.h"

using namespace Microsoft::WRL::Wrappers;

AssetLoader::AssetLoader(const std::wstring& assetRoot)
    : AssetRoot(assetRoot)
{
    assert(!assetRoot.empty());
    if (AssetRoot[AssetRoot.length() - 1] == L'\\')
    {
        AssetRoot.pop_back();
    }
}

AssetLoader::~AssetLoader()
{
}

HRESULT AssetLoader::LoadModel(const std::wstring& relativePath, std::shared_ptr<RenderVisual>* visual)
{
    assert(!relativePath.empty());
    assert(visual != nullptr);

    std::wstring fullPath = AssetRoot;
    if (relativePath[0] != L'\\')
    {
        fullPath += L'\\';
    }
    fullPath += relativePath;

    FileHandle file(CreateFile(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (!file.IsValid())
    {
        assert(false);
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    return S_OK;
}
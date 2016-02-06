#include "Precomp.h"
#include "AssetLoader.h"
#include "VertexFormats.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderVisual.h"

using namespace Microsoft::WRL::Wrappers;

#pragma pack(push)

// MODEL

#pragma pack(4)

struct ModelVertex
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT3 Tangent;
    XMFLOAT3 BiTangent;
    XMFLOAT2 TexCoord;
};

#pragma pack(1)

// Followed directly by all vertices, then all indices, then all objects
struct ModelHeader
{
    static const uint32_t ExpectedSignature = 'MODL';

    uint32_t Signature;
    uint32_t NumVertices;
    uint32_t NumIndices;
    uint32_t NumObjects;
};

// Followed directly by all parts
struct ModelObject
{
    char Name[128];
    uint32_t NumParts;
};

// Followed directly by all vertices
struct ModelPart
{
    wchar_t DiffuseTexture[256];
    wchar_t NormalTexture[256];
    wchar_t SpecularTexture[256];
    uint32_t StartIndex;
    uint32_t NumIndices;
};

#pragma pack(pop)

AssetLoader::AssetLoader(const ComPtr<ID3D11Device>& device, const std::wstring& assetRoot)
    : Device(device)
    , AssetRoot(assetRoot)
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

HRESULT AssetLoader::LoadModel(const std::wstring& relativePath, std::vector<std::shared_ptr<RenderVisual>>* visuals)
{
    assert(!relativePath.empty());
    assert(visuals != nullptr);

    visuals->clear();

    std::wstring fullPath = AssetRoot;
    if (relativePath[0] != L'\\')
    {
        fullPath += L'\\';
    }
    fullPath += relativePath;

    FileHandle modelFile(CreateFile(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (!modelFile.IsValid())
    {
        assert(false);
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    DWORD bytesRead = 0;
    ModelHeader header{};
    if (!ReadFile(modelFile.Get(), &header, sizeof(header), &bytesRead, nullptr))
    {
        assert(false);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (header.Signature != header.ExpectedSignature)
    {
        assert(false);
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    std::unique_ptr<ModelVertex[]> verts(new ModelVertex[header.NumVertices]);
    if (!ReadFile(modelFile.Get(), verts.get(), header.NumVertices * sizeof(ModelVertex), &bytesRead, nullptr))
    {
        assert(false);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    std::unique_ptr<uint32_t[]> indices(new uint32_t[header.NumIndices]);
    if (!ReadFile(modelFile.Get(), indices.get(), header.NumIndices * sizeof(uint32_t), &bytesRead, nullptr))
    {
        assert(false);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    std::unique_ptr<PositionNormalVertex[]> vertices(new PositionNormalVertex[header.NumVertices]);
    for (uint32_t i = 0; i < header.NumVertices; ++i)
    {
        vertices[i].Position = verts[i].Position;
        vertices[i].Normal = verts[i].Normal;
    }
    verts.reset();

    std::shared_ptr<VertexBuffer> vb = std::make_shared<VertexBuffer>();
    HRESULT hr = vb->Initialize(Device, VertexFormat::PositionNormal, vertices.get(), sizeof(PositionNormalVertex) * header.NumVertices);
    CHECKHR(hr);

    std::shared_ptr<IndexBuffer> ib = std::make_shared<IndexBuffer>();
    hr = ib->Initialize(Device, indices.get(), sizeof(uint32_t) * header.NumIndices);
    CHECKHR(hr);

    // Free up memory
    vertices.reset();
    indices.reset();

    // Load objects
    for (int iObj = 0; iObj < (int)header.NumObjects; ++iObj)
    {
        ModelObject obj{};
        if (!ReadFile(modelFile.Get(), &obj, sizeof(obj), &bytesRead, nullptr))
        {
            assert(false);
            return HRESULT_FROM_WIN32(GetLastError());
        }

        for (int iPart = 0; iPart < (int)obj.NumParts; ++iPart)
        {
            ModelPart part{};
            if (!ReadFile(modelFile.Get(), &part, sizeof(part), &bytesRead, nullptr))
            {
                assert(false);
                return HRESULT_FROM_WIN32(GetLastError());
            }

            std::shared_ptr<RenderVisual> visual = std::make_shared<RenderVisual>();
            hr = visual->Initialize(vb, ib, part.StartIndex, part.NumIndices);
            CHECKHR(hr);

            visuals->push_back(visual);
        }
    }

    return S_OK;
}
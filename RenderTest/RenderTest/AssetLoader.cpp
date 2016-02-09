#include "Precomp.h"
#include "AssetLoader.h"
#include "CoreGraphics/GraphicsDevice.h"
#include "CoreGraphics/VertexFormats.h"
#include "CoreGraphics/VertexBuffer.h"
#include "CoreGraphics/IndexBuffer.h"
#include "CoreGraphics/Texture.h"
#include "Scene/Visual.h"

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

#pragma pack(1)

// TEXTURE

// Followed immediately by raw texture data ready for uploading to GPU
struct TextureHeader
{
    static const uint32_t ExpectedSignature = 'TEX ';

    uint32_t Signature;
    uint32_t Width;
    uint32_t Height;
    uint32_t ArrayCount;
    uint32_t MipLevels;
    DXGI_FORMAT Format;
};

#pragma pack(pop)

AssetLoader::AssetLoader(const std::shared_ptr<GraphicsDevice>& graphics, const std::wstring& assetRoot)
    : Graphics(graphics)
    , AssetRoot(assetRoot)
{
    assert(!assetRoot.empty());
    if (AssetRoot[AssetRoot.length() - 1] != L'\\')
    {
        AssetRoot.push_back(L'\\');
    }
}

AssetLoader::~AssetLoader()
{
}

HRESULT AssetLoader::LoadModel(const std::wstring& relativePath, std::vector<std::shared_ptr<Visual>>* visuals)
{
    assert(!relativePath.empty());
    assert(visuals != nullptr);

    visuals->clear();

    std::wstring fullPath = AssetRoot + relativePath;

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

    std::unique_ptr<Basic3DVertex[]> vertices(new Basic3DVertex[header.NumVertices]);
    for (uint32_t i = 0; i < header.NumVertices; ++i)
    {
        vertices[i].Position = verts[i].Position;
        vertices[i].Normal = verts[i].Normal;
        vertices[i].Tangent = verts[i].Tangent;
        vertices[i].BiTangent = verts[i].BiTangent;
        vertices[i].TexCoord = verts[i].TexCoord;
    }
    verts.reset();

    std::shared_ptr<VertexBuffer> vb;
    HRESULT hr = Graphics->CreateVertexBuffer(VertexFormat::Basic3D, vertices.get(), sizeof(Basic3DVertex) * header.NumVertices, &vb);
    CHECKHR(hr);

    std::shared_ptr<IndexBuffer> ib;
    hr = Graphics->CreateIndexBuffer(indices.get(), sizeof(uint32_t) * header.NumIndices, &ib);
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

            std::shared_ptr<Visual> visual = std::make_shared<Visual>();
            hr = visual->Initialize(vb, ib, part.StartIndex, part.NumIndices);
            CHECKHR(hr);

            visuals->push_back(visual);

            if (part.DiffuseTexture[0] != 0)
            {
                std::wstring path = AssetRoot + part.DiffuseTexture;
                auto it = CachedTextures.find(path);
                if (it == CachedTextures.end())
                {
                    std::shared_ptr<Texture2D> texture;
                    hr = LoadTexture(path, &texture);
                    CHECKHR(hr);
                    CachedTextures[path] = texture;
                    visual->SetAlbedoTexture(texture);
                }
                else
                {
                    visual->SetAlbedoTexture(it->second);
                }
            }
            if (part.NormalTexture[0] != 0)
            {
                std::wstring path = AssetRoot + part.NormalTexture;
                auto it = CachedTextures.find(path);
                if (it == CachedTextures.end())
                {
                    std::shared_ptr<Texture2D> texture;
                    hr = LoadTexture(path, &texture);
                    CHECKHR(hr);
                    CachedTextures[path] = texture;
                    visual->SetNormalTexture(texture);
                }
                else
                {
                    visual->SetNormalTexture(it->second);
                }
            }
            if (part.SpecularTexture[0] != 0)
            {
                std::wstring path = AssetRoot + part.SpecularTexture;
                auto it = CachedTextures.find(path);
                if (it == CachedTextures.end())
                {
                    std::shared_ptr<Texture2D> texture;
                    hr = LoadTexture(path, &texture);
                    CHECKHR(hr);
                    CachedTextures[path] = texture;
                    visual->SetSpecularTexture(texture);
                }
                else
                {
                    visual->SetSpecularTexture(it->second);
                }
            }
        }
    }

    return S_OK;
}

HRESULT AssetLoader::LoadTexture(const std::wstring& relativePath, std::shared_ptr<Texture2D>* texture)
{
    assert(!relativePath.empty());
    assert(texture != nullptr);

    texture->reset();

    std::wstring fullPath = AssetRoot + relativePath;

    FileHandle texFile(CreateFile(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (!texFile.IsValid())
    {
        assert(false);
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    DWORD bytesRead{};
    uint32_t fileSize = GetFileSize(texFile.Get(), nullptr);

    TextureHeader texHeader{};
    if (!ReadFile(texFile.Get(), &texHeader, sizeof(texHeader), &bytesRead, nullptr))
    {
        assert(false);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (texHeader.Signature != TextureHeader::ExpectedSignature)
    {
        assert(false);
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    uint32_t pixelDataSize = fileSize - sizeof(TextureHeader);
    std::unique_ptr<uint8_t[]> pixelData(new uint8_t[pixelDataSize]);
    if (!ReadFile(texFile.Get(), pixelData.get(), pixelDataSize, &bytesRead, nullptr))
    {
        assert(false);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    D3D11_TEXTURE2D_DESC td{};
    td.ArraySize = texHeader.ArrayCount;
    td.Format = texHeader.Format;
#if USE_SRGB
    if (td.Format == DXGI_FORMAT_R8G8B8A8_UNORM)
    {
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    }
    else if (td.Format == DXGI_FORMAT_B8G8R8A8_UNORM)
    {
        td.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    }
#endif
    td.Width = texHeader.Width;
    td.Height = texHeader.Height;
    td.MipLevels = texHeader.MipLevels;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;

    HRESULT hr = Graphics->CreateTexture2D(td, pixelData.get(), texture);
    CHECKHR(hr);

    return S_OK;
}


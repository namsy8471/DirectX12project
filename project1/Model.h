#pragma once
#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <memory>

// DirectX 및 Microsoft 네임스페이스
using namespace Microsoft::WRL;
using namespace DirectX;

struct Vertex {
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT2 TexCoord;
};

class Model {
public:
    Model();
    ~Model();

    bool LoadFromFile(const std::string& filePath, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    void Render(ID3D12GraphicsCommandList* commandList);

private:
    // 메쉬 데이터
    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;

    // DirectX 12 리소스
    ComPtr<ID3D12Resource> mVertexBuffer;
    ComPtr<ID3D12Resource> mIndexBuffer;
    ComPtr<ID3D12Resource> mTexture;

    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
    D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

    // 로드 헬퍼 함수
    bool LoadMesh(const std::string& filePath);
    bool CreateBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    bool LoadTexture(const std::string& texturePath, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
};

#include "Model.h"
#include <stdexcept>
#include <fstream>
#include <iostream>
#include "D3D12Lite.h"
#include <d3d12.h>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

Model::Model() {}

Model::~Model() {}

bool Model::LoadFromFile(const std::string& filePath, ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    // 메쉬 데이터 로드
    if (!LoadMesh(filePath)) {
        std::cerr << "Failed to load mesh from file: " << filePath << std::endl;
        return false;
    }

    // DirectX 12 버퍼 생성
    if (!CreateBuffers(device, commandList)) {
        std::cerr << "Failed to create DirectX 12 buffers." << std::endl;
        return false;
    }

    // 텍스처 로드 (임의의 텍스처 파일 경로)
    std::string texturePath = "path/to/texture.png"; // 필요에 따라 수정
    if (!LoadTexture(texturePath, device, commandList)) {
        std::cerr << "Failed to load texture from file: " << texturePath << std::endl;
        return false;
    }

    return true;
}

bool Model::LoadMesh(const std::string& filePath)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals);

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    aiMesh* mesh = scene->mMeshes[0]; // 첫 번째 메쉬만 로드
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        if (mesh->mTextureCoords[0]) {
            vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }
        else {
            vertex.TexCoord = { 0.0f, 0.0f };
        }
        mVertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            mIndices.push_back(face.mIndices[j]);
        }
    }

    return true;
}

bool Model::CreateBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    // Vertex Buffer
    const UINT vertexBufferSize = static_cast<UINT>(mVertices.size() * sizeof(Vertex));
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&mVertexBuffer)));

    void* pVertexData = nullptr;
    ThrowIfFailed(mVertexBuffer->Map(0, nullptr, &pVertexData));
    memcpy(pVertexData, mVertices.data(), vertexBufferSize);
    mVertexBuffer->Unmap(0, nullptr);

    mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
    mVertexBufferView.SizeInBytes = vertexBufferSize;
    mVertexBufferView.StrideInBytes = sizeof(Vertex);

    // Index Buffer
    const UINT indexBufferSize = static_cast<UINT>(mIndices.size() * sizeof(uint32_t));
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&mIndexBuffer)));

    void* pIndexData = nullptr;
    ThrowIfFailed(mIndexBuffer->Map(0, nullptr, &pIndexData));
    memcpy(pIndexData, mIndices.data(), indexBufferSize);
    mIndexBuffer->Unmap(0, nullptr);

    mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
    mIndexBufferView.SizeInBytes = indexBufferSize;
    mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;

    return true;
}

bool Model::LoadTexture(const std::string& texturePath, ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    // 텍스처 로드 구현 (라이브러리 사용 가능, 예: DirectXTK)
    // 이 함수는 텍스처 로드 및 ID3D12Resource 생성 과정을 포함합니다.
    return true;
}

void Model::Render(ID3D12GraphicsCommandList* commandList)
{
    commandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
    commandList->IASetIndexBuffer(&mIndexBufferView);
    commandList->DrawIndexedInstanced(static_cast<UINT>(mIndices.size()), 1, 0, 0, 0);
}

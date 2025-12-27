//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/Dx12GeometryHandler.h>
#include <MaterialXRenderHlsl/Dx12Renderer.h>

#include <MaterialXRender/Mesh.h>

#include <d3dx12.h>

MATERIALX_NAMESPACE_BEGIN

//
// Dx12GeometryHandler methods
//

Dx12GeometryHandler::Dx12GeometryHandler() :
    _device(nullptr),
    _commandQueue(nullptr)
{
}

Dx12GeometryHandler::~Dx12GeometryHandler()
{
    _meshBuffers.clear();
}

Dx12MeshBuffer* Dx12GeometryHandler::createMeshBuffer(MeshPtr mesh)
{
    if (!mesh || !_device)
    {
        return nullptr;
    }

    auto it = _meshBuffers.find(mesh);
    if (it != _meshBuffers.end())
    {
        return it->second.get();
    }

    auto meshBuffer = Dx12MeshBuffer::create(_device, mesh);
    if (!meshBuffer)
    {
        return nullptr;
    }

    _meshBuffers[mesh] = meshBuffer;
    return meshBuffer.get();
}

Dx12MeshBuffer* Dx12GeometryHandler::getMeshBuffer(MeshPtr mesh)
{
    auto it = _meshBuffers.find(mesh);
    if (it != _meshBuffers.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void Dx12GeometryHandler::removeMeshBuffer(MeshPtr mesh)
{
    auto it = _meshBuffers.find(mesh);
    if (it != _meshBuffers.end())
    {
        _meshBuffers.erase(it);
    }
}

//
// Dx12MeshBuffer methods
//

std::shared_ptr<Dx12MeshBuffer> Dx12MeshBuffer::create(ID3D12Device* device, MeshPtr mesh)
{
    if (!device || !mesh)
    {
        return nullptr;
    }

    auto meshBuffer = std::shared_ptr<Dx12MeshBuffer>(new Dx12MeshBuffer(device, mesh));

    if (!meshBuffer->createVertexBuffer() || !meshBuffer->createIndexBuffer())
    {
        return nullptr;
    }

    return meshBuffer;
}

Dx12MeshBuffer::Dx12MeshBuffer(ID3D12Device* device, MeshPtr mesh) :
    _device(device),
    _mesh(mesh),
    _vertexStride(0),
    _indexCount(0)
{
    memset(&_vertexBufferView, 0, sizeof(_vertexBufferView));
    memset(&_indexBufferView, 0, sizeof(_indexBufferView));
}

Dx12MeshBuffer::~Dx12MeshBuffer()
{
}

bool Dx12MeshBuffer::createVertexBuffer()
{
    if (!_mesh)
    {
        return false;
    }

    // Calculate vertex data size
    // MaterialX meshes have: position (3 floats), normal (3 floats), texcoord (2 floats), tangent (3 floats)
    // Total: 11 floats per vertex = 44 bytes
    const size_t FLOATS_PER_VERTEX = 11;
    const size_t VERTEX_SIZE = FLOATS_PER_VERTEX * sizeof(float);

    size_t vertexCount = _mesh->getVertexCount();
    size_t vertexBufferSize = vertexCount * VERTEX_SIZE;

    if (vertexBufferSize == 0)
    {
        return false;
    }

    // Create vertex buffer
    HRESULT hr = _device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&_vertexBuffer));

    if (FAILED(hr))
    {
        return false;
    }

    // Create upload buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
    hr = _device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer));

    if (FAILED(hr))
    {
        return false;
    }

    // Copy vertex data to upload buffer
    void* mappedData = nullptr;
    uploadBuffer->Map(0, nullptr, &mappedData);
    if (mappedData)
    {
        float* vertexData = static_cast<float*>(mappedData);

        // Get mesh streams
        MeshStreamPtr positions = _mesh->getStream(MeshStream::POSITION_ATTRIBUTE);
        MeshStreamPtr normals = _mesh->getStream(MeshStream::NORMAL_ATTRIBUTE);
        MeshStreamPtr texcoords = _mesh->getStream(MeshStream::TEXCOORD_ATTRIBUTE);
        MeshStreamPtr tangents = _mesh->getStream(MeshStream::TANGENT_ATTRIBUTE);

        for (size_t i = 0; i < vertexCount; ++i)
        {
            // Position (3 floats)
            if (positions && i < positions->getData().size() / 3)
            {
                vertexData[0] = positions->getData()[i * 3 + 0];
                vertexData[1] = positions->getData()[i * 3 + 1];
                vertexData[2] = positions->getData()[i * 3 + 2];
            }
            else
            {
                vertexData[0] = 0.0f;
                vertexData[1] = 0.0f;
                vertexData[2] = 0.0f;
            }

            // Normal (3 floats)
            if (normals && i < normals->getData().size() / 3)
            {
                vertexData[3] = normals->getData()[i * 3 + 0];
                vertexData[4] = normals->getData()[i * 3 + 1];
                vertexData[5] = normals->getData()[i * 3 + 2];
            }
            else
            {
                vertexData[3] = 0.0f;
                vertexData[4] = 0.0f;
                vertexData[5] = 1.0f;
            }

            // Texcoord (2 floats)
            if (texcoords && i < texcoords->getData().size() / 2)
            {
                vertexData[6] = texcoords->getData()[i * 2 + 0];
                vertexData[7] = texcoords->getData()[i * 2 + 1];
            }
            else
            {
                vertexData[6] = 0.0f;
                vertexData[7] = 0.0f;
            }

            // Tangent (3 floats)
            if (tangents && i < tangents->getData().size() / 3)
            {
                vertexData[8] = tangents->getData()[i * 3 + 0];
                vertexData[9] = tangents->getData()[i * 3 + 1];
                vertexData[10] = tangents->getData()[i * 3 + 2];
            }
            else
            {
                vertexData[8] = 1.0f;
                vertexData[9] = 0.0f;
                vertexData[10] = 0.0f;
            }

            vertexData += FLOATS_PER_VERTEX;
        }

        uploadBuffer->Unmap(0, nullptr);
    }

    // Note: In a real implementation, you'd need to execute a command list to copy
    // from the upload buffer to the vertex buffer and transition the resource state.
    // For now, this is a placeholder structure.

    // Create vertex buffer view
    _vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
    _vertexBufferView.SizeInBytes = static_cast<UINT>(vertexBufferSize);
    _vertexBufferView.StrideInBytes = static_cast<UINT>(VERTEX_SIZE);
    _vertexStride = static_cast<UINT>(VERTEX_SIZE);

    return true;
}

bool Dx12MeshBuffer::createIndexBuffer()
{
    if (!_mesh)
    {
        return false;
    }

    // Calculate index data size
    size_t indexCount = 0;
    for (size_t i = 0; i < _mesh->getPartitionCount(); ++i)
    {
        MeshPartitionPtr partition = _mesh->getPartition(i);
        if (partition)
        {
            indexCount += partition->getIndices().size();
        }
    }

    _indexCount = static_cast<UINT>(indexCount);
    size_t indexBufferSize = indexCount * sizeof(unsigned int);

    if (indexBufferSize == 0)
    {
        return false;
    }

    // Create index buffer
    HRESULT hr = _device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&_indexBuffer));

    if (FAILED(hr))
    {
        return false;
    }

    // Create upload buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
    hr = _device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer));

    if (FAILED(hr))
    {
        return false;
    }

    // Copy index data to upload buffer
    void* mappedData = nullptr;
    uploadBuffer->Map(0, nullptr, &mappedData);
    if (mappedData)
    {
        unsigned int* indexData = static_cast<unsigned int*>(mappedData);
        size_t offset = 0;

        for (size_t i = 0; i < _mesh->getPartitionCount(); ++i)
        {
            MeshPartitionPtr partition = _mesh->getPartition(i);
            if (partition)
            {
                const MeshIndexBuffer& indices = partition->getIndices();
                memcpy(indexData + offset, indices.data(), indices.size() * sizeof(unsigned int));
                offset += indices.size();
            }
        }

        uploadBuffer->Unmap(0, nullptr);
    }

    // Note: In a real implementation, you'd need to execute a command list to copy
    // from the upload buffer to the index buffer and transition the resource state.
    // For now, this is a placeholder structure.

    // Create index buffer view
    _indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
    _indexBufferView.SizeInBytes = static_cast<UINT>(indexBufferSize);
    _indexBufferView.Format = DXGI_FORMAT_R32_UINT;

    return true;
}

MATERIALX_NAMESPACE_END

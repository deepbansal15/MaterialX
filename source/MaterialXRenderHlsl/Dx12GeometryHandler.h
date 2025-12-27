//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_DX12GEOMETRYHANDLER_H
#define MATERIALX_DX12GEOMETRYHANDLER_H

#include <MaterialXRenderHlsl/Export.h>

#include <MaterialXRender/GeometryHandler.h>

#include <d3d12.h>
#include <wrl.h>

#include <memory>
#include <unordered_map>

MATERIALX_NAMESPACE_BEGIN

class Dx12MeshBuffer;

/// Shared pointer to a Dx12GeometryHandler
using Dx12GeometryHandlerPtr = std::shared_ptr<class Dx12GeometryHandler>;

/// @class Dx12GeometryHandler
/// A DirectX 12 geometry handler implementation for MaterialX
class MX_RENDERHLSL_API Dx12GeometryHandler : public GeometryHandler
{
  public:
    /// Create a new Dx12GeometryHandler
    static Dx12GeometryHandlerPtr create()
    {
        return Dx12GeometryHandlerPtr(new Dx12GeometryHandler());
    }

    /// Destructor
    ~Dx12GeometryHandler() override;

    /// Set the D3D12 device for buffer creation
    void setDevice(ID3D12Device* device) { _device = device; }

    /// Set the command queue for buffer uploads
    void setCommandQueue(ID3D12CommandQueue* commandQueue) { _commandQueue = commandQueue; }

    /// Create a mesh buffer for a given mesh
    Dx12MeshBuffer* createMeshBuffer(MeshPtr mesh);

    /// Get the mesh buffer for a given mesh
    Dx12MeshBuffer* getMeshBuffer(MeshPtr mesh);

    /// Remove a mesh buffer
    void removeMeshBuffer(MeshPtr mesh);

  protected:
    /// Constructor
    Dx12GeometryHandler();

    /// D3D12 device for buffer creation
    ID3D12Device* _device;

    /// D3D12 command queue for buffer uploads
    ID3D12CommandQueue* _commandQueue;

    /// Map of mesh to mesh buffer
    std::unordered_map<MeshPtr, std::shared_ptr<Dx12MeshBuffer>> _meshBuffers;
};

/// @class Dx12MeshBuffer
/// A DirectX 12 mesh buffer implementation
class MX_RENDERHLSL_API Dx12MeshBuffer
{
  public:
    /// Create a new Dx12MeshBuffer
    static std::shared_ptr<Dx12MeshBuffer> create(ID3D12Device* device, MeshPtr mesh);

    /// Destructor
    ~Dx12MeshBuffer();

    /// Get the vertex buffer
    ID3D12Resource* getVertexBuffer() const { return _vertexBuffer.Get(); }

    /// Get the vertex buffer view
    const D3D12_VERTEX_BUFFER_VIEW& getVertexBufferView() const { return _vertexBufferView; }

    /// Get the index buffer
    ID3D12Resource* getIndexBuffer() const { return _indexBuffer.Get(); }

    /// Get the index buffer view
    const D3D12_INDEX_BUFFER_VIEW& getIndexBufferView() const { return _indexBufferView; }

    /// Get the vertex stride
    UINT getVertexStride() const { return _vertexStride; }

    /// Get the index count
    UINT getIndexCount() const { return _indexCount; }

    /// Get the mesh
    MeshPtr getMesh() const { return _mesh; }

  protected:
    /// Constructor
    Dx12MeshBuffer(ID3D12Device* device, MeshPtr mesh);

    /// Create vertex buffer
    bool createVertexBuffer();

    /// Create index buffer
    bool createIndexBuffer();

    ID3D12Device* _device;
    MeshPtr _mesh;

    Microsoft::WRL::ComPtr<ID3D12Resource> _vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
    UINT _vertexStride;

    Microsoft::WRL::ComPtr<ID3D12Resource> _indexBuffer;
    D3D12_INDEX_BUFFER_VIEW _indexBufferView;
    UINT _indexCount;
};

MATERIALX_NAMESPACE_END

#endif

//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_DX12RENDERER_H
#define MATERIALX_DX12RENDERER_H

#include <MaterialXRenderHlsl/Export.h>

#include <MaterialXRender/ShaderRenderer.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

MATERIALX_NAMESPACE_BEGIN

using Dx12RendererPtr = shared_ptr<class Dx12Renderer>;

/// @class Dx12Renderer
/// DirectX 12 implementation of ShaderRenderer
class MX_RENDERHLSL_API Dx12Renderer : public ShaderRenderer
{
  public:
    /// Create a new Dx12Renderer
    static Dx12RendererPtr create();

    /// Destructor
    ~Dx12Renderer();

    /// Initialize the renderer with the given device and command queue
    void initialize(void* device, void* commandQueue) override;

    /// Create an image handler for DirectX 12
    ImageHandlerPtr createImageHandler() override;

    /// Get the current Direct3D 12 device
    ID3D12Device* getDevice() const { return _device.Get(); }

    /// Get the current Direct3D 12 command queue
    ID3D12CommandQueue* getCommandQueue() const { return _commandQueue.Get(); }

    /// Get the Direct3D 12 root signature
    ID3D12RootSignature* getRootSignature() const { return _rootSignature.Get(); }

    /// Create a command list for texture uploads
    /// Returns S_OK on success, the caller must call Release on the command list
    HRESULT createCommandList(ID3D12GraphicsCommandList** commandList);

  protected:
    /// Constructor
    Dx12Renderer();

    /// Create the Direct3D 12 device and command queue
    void createDevice();

    /// Create the root signature for shaders
    void createRootSignature();

    Microsoft::WRL::ComPtr<ID3D12Device> _device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> _commandQueue;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
    Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
    UINT64 _fenceValue;
    HANDLE _fenceEvent;
};

MATERIALX_NAMESPACE_END

#endif

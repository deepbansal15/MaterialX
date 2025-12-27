//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_DX12STATE_H
#define MATERIALX_DX12STATE_H

#include <MaterialXRenderHlsl/Export.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

MATERIALX_NAMESPACE_BEGIN

/// @class Dx12State
/// Manages DirectX 12 device, command queue, and synchronization objects
class MX_RENDERHLSL_API Dx12State
{
  public:
    /// Create a new Dx12State
    static Dx12StatePtr create();

    /// Destructor
    ~Dx12State();

    /// Initialize the DirectX 12 device and command queue
    /// @param device Optional external device to use
    /// @param commandQueue Optional external command queue to use
    bool initialize(ID3D12Device* device = nullptr, ID3D12CommandQueue* commandQueue = nullptr);

    /// Get the D3D12 device
    ID3D12Device* getDevice() const { return _device.Get(); }

    /// Get the command queue
    ID3D12CommandQueue* getCommandQueue() const { return _commandQueue.Get(); }

    /// Get the command allocator
    ID3D12CommandAllocator* getCommandAllocator() const { return _commandAllocator.Get(); }

    /// Get the fence
    ID3D12Fence* getFence() const { return _fence.Get(); }

    /// Get the current fence value
    UINT64 getFenceValue() const { return _fenceValue; }

    /// Increment the fence value
    void incrementFenceValue() { ++_fenceValue; }

    /// Signal the fence
    HRESULT signalFence();

    /// Wait for the fence to reach a given value
    HRESULT waitForFence(UINT64 value);

    /// Get the fence event handle
    HANDLE getFenceEvent() const { return _fenceEvent; }

    /// Check if the state is initialized
    bool isInitialized() const { return _initialized; }

  protected:
    /// Constructor
    Dx12State();

    /// Create the D3D12 device
    bool createDevice();

    /// Create the command queue
    bool createCommandQueue();

    /// Create the command allocator
    bool createCommandAllocator();

    /// Create the fence and event
    bool createFence();

  private:
    Microsoft::WRL::ComPtr<ID3D12Device> _device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> _commandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
    UINT64 _fenceValue;
    HANDLE _fenceEvent;
    bool _initialized;
};

MATERIALX_NAMESPACE_END

#endif

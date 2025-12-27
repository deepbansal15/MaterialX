//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/Dx12State.h>

#include <stdexcept>

MATERIALX_NAMESPACE_BEGIN

Dx12State::Dx12State() :
    _fenceValue(0),
    _fenceEvent(nullptr),
    _initialized(false)
{
}

Dx12State::~Dx12State()
{
    if (_fenceEvent)
    {
        CloseHandle(_fenceEvent);
        _fenceEvent = nullptr;
    }
}

Dx12StatePtr Dx12State::create()
{
    return Dx12StatePtr(new Dx12State());
}

bool Dx12State::initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue)
{
    if (_initialized)
    {
        return true;
    }

    if (device && commandQueue)
    {
        // Use provided device and command queue
        _device = device;
        _commandQueue = commandQueue;
    }
    else
    {
        // Create our own device
        if (!createDevice())
        {
            return false;
        }

        if (!createCommandQueue())
        {
            return false;
        }
    }

    if (!createCommandAllocator())
    {
        return false;
    }

    if (!createFence())
    {
        return false;
    }

    _initialized = true;
    return true;
}

bool Dx12State::createDevice()
{
    HRESULT hr = D3D12CreateDevice(
        nullptr,                    // Use default adapter
        D3D_FEATURE_LEVEL_11_0,     // Minimum feature level
        IID_PPV_ARGS(&_device));

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Direct3D 12 device");
    }

    return true;
}

bool Dx12State::createCommandQueue()
{
    if (!_device)
    {
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    HRESULT hr = _device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Direct3D 12 command queue");
    }

    return true;
}

bool Dx12State::createCommandAllocator()
{
    if (!_device)
    {
        return false;
    }

    HRESULT hr = _device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&_commandAllocator));

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Direct3D 12 command allocator");
    }

    return true;
}

bool Dx12State::createFence()
{
    if (!_device)
    {
        return false;
    }

    HRESULT hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Direct3D 12 fence");
    }

    _fenceValue = 0;

    _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!_fenceEvent)
    {
        throw std::runtime_error("Failed to create fence event");
    }

    return true;
}

HRESULT Dx12State::signalFence()
{
    if (!_commandQueue || !_fence)
    {
        return E_FAIL;
    }

    _fenceValue++;
    return _commandQueue->Signal(_fence.Get(), _fenceValue);
}

HRESULT Dx12State::waitForFence(UINT64 value)
{
    if (!_fence || !_fenceEvent)
    {
        return E_FAIL;
    }

    // Check if the fence has already been signaled
    if (_fence->GetCompletedValue() < value)
    {
        // Wait for the fence to be signaled
        HRESULT hr = _fence->SetEventOnCompletion(value, _fenceEvent);
        if (FAILED(hr))
        {
            return hr;
        }

        WaitForSingleObject(_fenceEvent, INFINITE);
    }

    return S_OK;
}

MATERIALX_NAMESPACE_END

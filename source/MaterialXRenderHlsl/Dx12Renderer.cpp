//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/Dx12Renderer.h>

#include <MaterialXRender/Image.h>
#include <MaterialXRenderHlsl/Dx12TextureHandler.h>

#include <stdexcept>

MATERIALX_NAMESPACE_BEGIN

Dx12Renderer::Dx12Renderer() :
    _device(nullptr),
    _commandQueue(nullptr),
    _fenceValue(0)
{
}

Dx12Renderer::~Dx12Renderer()
{
    if (_fenceEvent)
    {
        CloseHandle(_fenceEvent);
        _fenceEvent = nullptr;
    }
}

Dx12RendererPtr Dx12Renderer::create()
{
    return std::make_shared<Dx12Renderer>();
}

void Dx12Renderer::createDevice()
{
    // Create D3D12 device
    HRESULT hr = D3D12CreateDevice(
        nullptr,                    // Use default adapter
        D3D_FEATURE_LEVEL_11_0,     // Minimum feature level
        IID_PPV_ARGS(&_device));

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Direct3D 12 device");
    }

    // Create command queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    hr = _device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Direct3D 12 command queue");
    }

    // Create fence for synchronization
    hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create Direct3D 12 fence");
    }

    // Create fence event
    _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!_fenceEvent)
    {
        throw std::runtime_error("Failed to create fence event");
    }
}

void Dx12Renderer::createRootSignature()
{
    // Define a simple root signature with one descriptor table
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 0;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1_0,
        &signatureBlob,
        &errorBlob);

    if (FAILED(hr))
    {
        std::string errorMessage = "Failed to serialize root signature";
        if (errorBlob)
        {
            errorMessage += ": " + std::string(static_cast<char*>(errorBlob->GetBufferPointer()));
        }
        throw std::runtime_error(errorMessage);
    }

    hr = _device->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&_rootSignature));

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create root signature");
    }
}

void Dx12Renderer::initialize(void* device, void* commandQueue)
{
    if (device && commandQueue)
    {
        // Use provided device and command queue
        _device = static_cast<ID3D12Device*>(device);
        _commandQueue = static_cast<ID3D12CommandQueue*>(commandQueue);
    }
    else
    {
        // Create our own device
        createDevice();
    }

    createRootSignature();
}

ImageHandlerPtr Dx12Renderer::createImageHandler()
{
    return Dx12TextureHandler::create();
}

MATERIALX_NAMESPACE_END

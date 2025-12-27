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
    // Define root parameters for the shader
    // MaterialX shaders typically need:
    // - Constant buffer for transformation matrices (b0)
    // - Constant buffer for material parameters (b1)
    // - Descriptor table for textures (t0-tN)
    // - Descriptor table for samplers (s0-sN)

    D3D12_ROOT_PARAMETER rootParameters[3];
    D3D12_DESCRIPTOR_RANGE descriptorRanges[2];

    // Descriptor range for textures (shader resource views)
    descriptorRanges[0].BaseShaderRegister = 0;
    descriptorRanges[0].NumDescriptors = 16;  // Support up to 16 textures
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

    // Descriptor range for samplers
    descriptorRanges[1].BaseShaderRegister = 0;
    descriptorRanges[1].NumDescriptors = 8;  // Support up to 8 samplers
    descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;

    // Root parameter 0: Descriptor table for textures
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Root parameter 1: Descriptor table for samplers
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Root parameter 2: 32-bit constants for simple uniform values
    // Register b0, space 0
    D3D12_ROOT_CONSTANTS constants = {};
    constants.ShaderRegister = 0;
    constants.RegisterSpace = 0;
    constants.Num32BitValues = 16;  // 16 floats (4x4 matrix) for MVP
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParameters[2].Constants = constants;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Define a root signature with the parameters
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 3;
    rootSignatureDesc.pParameters = rootParameters;
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

HRESULT Dx12Renderer::createCommandList(ID3D12GraphicsCommandList** commandList)
{
    if (!_device)
    {
        return E_FAIL;
    }

    return _device->CreateCommandList(
        0,  // Node mask
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        nullptr,  // Allocator
        nullptr,  // Initial pipeline state (can be set later)
        IID_PPV_ARGS(commandList));
}

MATERIALX_NAMESPACE_END

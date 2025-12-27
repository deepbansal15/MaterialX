//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/Dx12Framebuffer.h>

#include <MaterialXRender/Image.h>

#include <d3dx12.h>

MATERIALX_NAMESPACE_BEGIN

//
// Dx12Framebuffer methods
//

Dx12FramebufferPtr Dx12Framebuffer::create(ID3D12Device* device,
                                           unsigned int width, unsigned int height,
                                           unsigned int channelCount,
                                           Image::BaseType baseType,
                                           ID3D12Resource* colorTexture,
                                           bool encodeSrgb,
                                           DXGI_FORMAT pixelFormat)
{
    return Dx12FramebufferPtr(new Dx12Framebuffer(device,
                                                  width, height,
                                                  channelCount,
                                                  baseType,
                                                  colorTexture,
                                                  encodeSrgb,
                                                  pixelFormat));
}

Dx12Framebuffer::Dx12Framebuffer(ID3D12Device* device,
                                 unsigned int width, unsigned int height,
                                 unsigned int channelCount,
                                 Image::BaseType baseType,
                                 ID3D12Resource* colorTexture,
                                 bool encodeSrgb,
                                 DXGI_FORMAT pixelFormat) :
    _width(0),
    _height(0),
    _channelCount(channelCount),
    _baseType(baseType),
    _encodeSrgb(encodeSrgb),
    _device(device),
    _colorTexture(colorTexture),
    _depthTexture(nullptr),
    _colorTextureOwned(false)
{
    resize(width, height, true, pixelFormat, colorTexture);
}

Dx12Framebuffer::~Dx12Framebuffer()
{
}

DXGI_FORMAT Dx12Framebuffer::mapFormatToDXGI(Image::BaseType baseType, unsigned int channelCount, bool encodeSrgb)
{
    switch (baseType)
    {
        case Image::BaseType::FLOAT:
            if (channelCount == 1) return DXGI_FORMAT_R32_FLOAT;
            else if (channelCount == 2) return DXGI_FORMAT_R32G32_FLOAT;
            else if (channelCount == 3) return DXGI_FORMAT_R32G32B32_FLOAT;
            else if (channelCount == 4) return DXGI_FORMAT_R32G32B32A32_FLOAT;
            break;
        case Image::BaseType::HALF:
            if (channelCount == 1) return DXGI_FORMAT_R16_FLOAT;
            else if (channelCount == 2) return DXGI_FORMAT_R16G16_FLOAT;
            else if (channelCount == 4) return DXGI_FORMAT_R16G16B16A16_FLOAT;
            break;
        case Image::BaseType::UBYTE:
            if (channelCount == 1) return encodeSrgb ? DXGI_FORMAT_R8_UNORM_SRGB : DXGI_FORMAT_R8_UNORM;
            else if (channelCount == 2) return encodeSrgb ? DXGI_FORMAT_R8G8_UNORM_SRGB : DXGI_FORMAT_R8G8_UNORM;
            else if (channelCount == 3) return encodeSrgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
            else if (channelCount == 4) return encodeSrgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
    }
    return DXGI_FORMAT_R8G8B8A8_UNORM;
}

void Dx12Framebuffer::resize(unsigned int width, unsigned int height, bool forceRecreate,
                             DXGI_FORMAT pixelFormat,
                             ID3D12Resource* extColorTexture)
{
    if (width * height <= 0)
    {
        return;
    }
    if (width != _width || _height != height || forceRecreate)
    {
        // Convert texture format to DXGI
        if (pixelFormat == DXGI_FORMAT_UNKNOWN)
        {
            pixelFormat = mapFormatToDXGI(_baseType, _channelCount, _encodeSrgb);
        }

        // Create RTV heap
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 1;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;

        HRESULT hr = _device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&_rtvHeap));
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to create RTV descriptor heap");
        }

        _colorCPUDescriptor = _rtvHeap->GetCPUDescriptorHandleForHeapStart();

        // Create DSV heap
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NodeMask = 0;

        hr = _device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_dsvHeap));
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to create DSV descriptor heap");
        }

        _depthCPUDescriptor = _dsvHeap->GetCPUDescriptorHandleForHeapStart();

        // Create color texture
        if (extColorTexture == nullptr)
        {
            D3D12_RESOURCE_DESC texDesc = {};
            texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            texDesc.Width = width;
            texDesc.Height = height;
            texDesc.DepthOrArraySize = 1;
            texDesc.MipLevels = 1;
            texDesc.Format = pixelFormat;
            texDesc.SampleDesc.Count = 1;
            texDesc.SampleDesc.Quality = 0;
            texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

            D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

            hr = _device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &texDesc,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                nullptr,
                IID_PPV_ARGS(&_colorTexture));

            if (FAILED(hr))
            {
                throw std::runtime_error("Failed to create color texture");
            }

            _colorTextureOwned = true;
        }
        else
        {
            _colorTexture = extColorTexture;
            _colorTextureOwned = false;
        }

        // Create RTV
        _device->CreateRenderTargetView(_colorTexture.Get(), nullptr, _colorCPUDescriptor);

        // Create depth stencil texture
        D3D12_RESOURCE_DESC depthDesc = {};
        depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthDesc.Width = width;
        depthDesc.Height = height;
        depthDesc.DepthOrArraySize = 1;
        depthDesc.MipLevels = 1;
        depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.SampleDesc.Quality = 0;
        depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_HEAP_PROPERTIES depthHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        D3D12_CLEAR_VALUE depthClearValue = {};
        depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        depthClearValue.DepthStencil.Depth = 1.0f;
        depthClearValue.DepthStencil.Stencil = 0;

        hr = _device->CreateCommittedResource(
            &depthHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &depthDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthClearValue,
            IID_PPV_ARGS(&_depthTexture));

        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to create depth stencil texture");
        }

        // Create DSV
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        _device->CreateDepthStencilView(_depthTexture.Get(), &dsvDesc, _depthCPUDescriptor);

        _width = width;
        _height = height;
    }
}

ImagePtr Dx12Framebuffer::getColorImage(ID3D12CommandQueue* commandQueue, ImagePtr image)
{
    if (!image)
    {
        image = Image::create(_width, _height, _channelCount, _baseType);
        image->createResourceBuffer();
    }

    if (!commandQueue)
    {
        return image;
    }

    // Create a command list for the copy operation
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    HRESULT hr = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    if (FAILED(hr))
    {
        return image;
    }

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
    hr = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    if (FAILED(hr))
    {
        return image;
    }

    // Create a staging buffer
    unsigned int bytesPerPixel = _channelCount * Image::getBaseTypeSize(_baseType);
    unsigned int rowPitch = (_width * bytesPerPixel + 255) & ~255;
    unsigned int totalSize = rowPitch * _height;

    D3D12_RESOURCE_DESC stagingDesc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);
    D3D12_HEAP_PROPERTIES stagingHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);

    Microsoft::WRL::ComPtr<ID3D12Resource> stagingBuffer;
    hr = _device->CreateCommittedResource(
        &stagingHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &stagingDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&stagingBuffer));

    if (FAILED(hr))
    {
        return image;
    }

    // Transition color texture to copy source state
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        _colorTexture.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList->ResourceBarrier(1, &barrier);

    // Copy texture to staging buffer
    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = stagingBuffer.Get();
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dstLocation.PlacedFootprint.Footprint.Format = _colorTexture->GetDesc().Format;
    dstLocation.PlacedFootprint.Footprint.Width = _width;
    dstLocation.PlacedFootprint.Footprint.Height = _height;
    dstLocation.PlacedFootprint.Footprint.Depth = 1;
    dstLocation.PlacedFootprint.Footprint.RowPitch = rowPitch;

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = _colorTexture.Get();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = 0;

    commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

    // Transition back to render target state
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        _colorTexture.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);

    commandList->Close();

    // Execute the command list
    ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, ppCommandLists);

    // Wait for completion
    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (SUCCEEDED(hr))
    {
        HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (fenceEvent)
        {
            UINT64 fenceValue = 1;
            commandQueue->Signal(fence.Get(), fenceValue);
            if (fence->GetCompletedValue() < fenceValue)
            {
                fence->SetEventOnCompletion(fenceValue, fenceEvent);
                WaitForSingleObject(fenceEvent, INFINITE);
            }
            CloseHandle(fenceEvent);
        }
    }

    // Map the staging buffer and copy data
    void* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, totalSize };
    hr = stagingBuffer->Map(0, &readRange, &mappedData);
    if (SUCCEEDED(hr) && mappedData)
    {
        memcpy(image->getResourceBuffer(), mappedData, totalSize);
        stagingBuffer->Unmap(0, nullptr);
    }

    return image;
}

MATERIALX_NAMESPACE_END

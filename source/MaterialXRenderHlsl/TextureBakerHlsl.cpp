//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
#include <MaterialXRenderHlsl/TextureBakerHlsl.h>

#include <MaterialXRender/Image.h>
#include <MaterialXRender/Renderer.h>
#include <MaterialXRenderHlsl/Dx12Renderer.h>
#include <MaterialXRenderHlsl/Dx12TextureHandler.h>

#include <MaterialXGenHw/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

TextureBakerHlsl::TextureBakerHlsl(RendererPtr renderer) :
    TextureBaker(renderer),
    _dx12Device(nullptr),
    _dx12CommandList(nullptr),
    _dx12CommandAllocator(nullptr),
    _dx12DescriptorHeap(nullptr),
    _descriptorHeapSize(0)
{
}

TextureBakerHlsl::~TextureBakerHlsl()
{
    // Release D3D12 resources
    if (_dx12DescriptorHeap)
    {
        _dx12DescriptorHeap->Release();
        _dx12DescriptorHeap = nullptr;
    }
    if (_dx12CommandAllocator)
    {
        _dx12CommandAllocator->Release();
        _dx12CommandAllocator = nullptr;
    }
}

TextureBakerHlslPtr TextureBakerHlsl::create(RendererPtr renderer)
{
    return TextureBakerHlslPtr(new TextureBakerHlsl(renderer));
}

bool TextureBakerHlsl::initialize()
{
    if (!TextureBaker::initialize())
    {
        return false;
    }

    // Initialize D3D12 resources if hardware rendering is requested
    if (_renderer->hwRequested())
    {
        Dx12Renderer* dx12Renderer = dynamic_cast<Dx12Renderer*>(_renderer.get());
        if (!dx12Renderer)
        {
            return false;
        }

        _dx12Device = dx12Renderer->getDevice();
        if (!_dx12Device)
        {
            return false;
        }

        // Create command allocator
        HRESULT hr = _dx12Device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&_dx12CommandAllocator));
        if (FAILED(hr))
        {
            return false;
        }

        // Create command list
        hr = dx12Renderer->createCommandList(&_dx12CommandList);
        if (FAILED(hr) || !_dx12CommandList)
        {
            return false;
        }

        // Create descriptor heap for shader resource views
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.NumDescriptors = 128;  // Support up to 128 textures
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heapDesc.NodeMask = 0;

        hr = _dx12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_dx12DescriptorHeap));
        if (FAILED(hr))
        {
            return false;
        }

        _descriptorHeapSize = _dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    return true;
}

bool TextureBakerHlsl::createTextureFromFile(const string& resourceId, const string& filePath, bool verticalFlip)
{
    if (_renderer->hwRequested())
    {
        return createDx12Texture(resourceId, createImage(filePath, verticalFlip));
    }
    return TextureBaker::createTextureFromFile(resourceId, filePath, verticalFlip);
}

bool TextureBakerHlsl::createTextureFromImage(const string& resourceId, const ImagePtr image)
{
    if (_renderer->hwRequested())
    {
        return createDx12Texture(resourceId, image);
    }
    return TextureBaker::createTextureFromImage(resourceId, image);
}

bool TextureBakerHlsl::createDx12Texture(const string& resourceId, const ImagePtr image)
{
    if (!image || !_dx12Device || !_dx12CommandList)
    {
        return false;
    }

    // Check if we already have a texture with this resource ID
    auto it = _textures.find(resourceId);
    if (it != _textures.end())
    {
        return true; // Already exists
    }

    // Get image properties
    unsigned int width = image->getWidth();
    unsigned int height = image->getHeight();
    Image::BaseType baseType = image->getBaseType();
    unsigned int channelCount = image->getChannelCount();

    // Determine DXGI format based on image properties
    DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
    switch (baseType)
    {
        case Image::BaseType::FLOAT:
            if (channelCount == 1) dxgiFormat = DXGI_FORMAT_R32_FLOAT;
            else if (channelCount == 3) dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
            else if (channelCount == 4) dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
            break;
        case Image::BaseType::HALF:
            if (channelCount == 1) dxgiFormat = DXGI_FORMAT_R16_FLOAT;
            else if (channelCount == 4) dxgiFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
            break;
        case Image::BaseType::UBYTE:
            if (channelCount == 1) dxgiFormat = DXGI_FORMAT_R8_UNORM;
            else if (channelCount == 3) dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            else if (channelCount == 4) dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
    }

    if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
    {
        return false;
    }

    // Create texture description
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Format = dxgiFormat;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Create the texture resource
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
    HRESULT hr = _dx12Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&textureResource));

    if (FAILED(hr))
    {
        return false;
    }

    // Get the image data
    const unsigned char* imageData = image->getBuffer();
    if (!imageData)
    {
        return false;
    }

    // Calculate row pitch and total size
    unsigned int bytesPerPixel = image->getBytesPerPixel();
    unsigned int rowPitch = (width * bytesPerPixel + 255) & ~255;
    unsigned int totalSize = rowPitch * height;

    // Create upload buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
    CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);

    hr = _dx12Device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &uploadBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer));

    if (FAILED(hr))
    {
        return false;
    }

    // Copy data to upload buffer
    void* mappedData = nullptr;
    uploadBuffer->Map(0, nullptr, &mappedData);
    if (mappedData)
    {
        memcpy(mappedData, imageData, totalSize);
        uploadBuffer->Unmap(0, nullptr);
    }

    // Create texture upload command
    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = textureResource.Get();
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = uploadBuffer.Get();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.PlacedFootprint.Footprint.Format = dxgiFormat;
    srcLocation.PlacedFootprint.Footprint.Width = width;
    srcLocation.PlacedFootprint.Footprint.Height = height;
    srcLocation.PlacedFootprint.Footprint.Depth = 1;
    srcLocation.PlacedFootprint.Footprint.RowPitch = rowPitch;

    _dx12CommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

    // Transition resource to shader read state
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        textureResource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    _dx12CommandList->ResourceBarrier(1, &barrier);

    // Create shader resource view
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = dxgiFormat;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    // Allocate descriptor from heap
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = _dx12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += _textures.size() * _descriptorHeapSize;

    _dx12Device->CreateShaderResourceView(textureResource.Get(), &srvDesc, cpuHandle);

    // Store the texture resource
    _textures[resourceId] = textureResource.Detach();

    return true;
}

bool TextureBakerHlsl::bake2DTexture(MaterialPtr material,
                                      const string& shaderName,
                                      unsigned int width,
                                      unsigned int height,
                                      const string& type,
                                      const string& filename)
{
    // Delegate to base implementation for file baking
    return TextureBaker::bake2DTexture(material, shaderName, width, height, type, filename);
}

MATERIALX_NAMESPACE_END

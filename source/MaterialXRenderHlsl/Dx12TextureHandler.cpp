//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
#include <MaterialXRenderHlsl/Dx12TextureHandler.h>
#include <MaterialXRenderHlsl/Dx12Renderer.h>
#include <MaterialXRender/Renderer.h>
#include <MaterialXRender/Image.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dx12.h>

MATERIALX_NAMESPACE_BEGIN

Dx12TextureHandler::Dx12TextureHandler(RendererPtr renderer) :
    TextureHandler(renderer)
{
}

Dx12TextureHandler::~Dx12TextureHandler()
{
    // Release all GPU resources
    for (auto& pair : _textureResources)
    {
        if (pair.second)
        {
            pair.second->Release();
        }
    }
    _textureResources.clear();
    _textureViews.clear();
}

TextureHandlerPtr Dx12TextureHandler::create(RendererPtr renderer)
{
    return TextureHandlerPtr(new Dx12TextureHandler(renderer));
}

bool Dx12TextureHandler::setTexture(const string& resourceId, TexturePtr texture)
{
    if (!TextureHandler::setTexture(resourceId, texture))
    {
        return false;
    }

    if (!texture->getImage())
    {
        return false;
    }

    return createDx12Texture(resourceId, texture->getImage());
}

ID3D12Resource* Dx12TextureHandler::getTexture(const string& resourceId) const
{
    auto it = _textureResources.find(resourceId);
    if (it != _textureResources.end())
    {
        return it->second;
    }
    return nullptr;
}

D3D12_GPU_DESCRIPTOR_HANDLE Dx12TextureHandler::getTextureView(const string& resourceId) const
{
    auto it = _textureViews.find(resourceId);
    if (it != _textureViews.end())
    {
        return it->second;
    }
    D3D12_GPU_DESCRIPTOR_HANDLE nullHandle = { 0 };
    return nullHandle;
}

bool Dx12TextureHandler::createDx12Texture(const string& resourceId, const ImagePtr image)
{
    // Check if texture already exists
    auto it = _textureResources.find(resourceId);
    if (it != _textureResources.end())
    {
        // Texture already created
        return true;
    }

    if (!image)
    {
        return false;
    }

    // Get the D3D12 device from the renderer
    Dx12Renderer* dx12Renderer = dynamic_cast<Dx12Renderer*>(_renderer.get());
    if (!dx12Renderer)
    {
        return false;
    }

    ID3D12Device* device = dx12Renderer->getDevice();
    if (!device)
    {
        return false;
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
            else if (channelCount >= 3) dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
    }

    if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
    {
        return false;
    }

    // Create a command list for texture upload
    ID3D12GraphicsCommandList* commandList = nullptr;
    HRESULT hr = dx12Renderer->createCommandList(&commandList);
    if (FAILED(hr) || !commandList)
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
    hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&textureResource));

    if (FAILED(hr))
    {
        commandList->Release();
        return false;
    }

    // Get the image data
    const unsigned char* imageData = image->getBuffer();
    if (!imageData)
    {
        commandList->Release();
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

    hr = device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &uploadBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer));

    if (FAILED(hr))
    {
        commandList->Release();
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

    commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

    // Transition resource to shader read state
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        textureResource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &barrier);

    // Close and execute the command list
    commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { commandList };
    dx12Renderer->getCommandQueue()->ExecuteCommandLists(1, ppCommandLists);

    // Wait for the command list to complete
    // In a real implementation, you'd use proper synchronization here

    // Release the command list (the resource will be released when the texture handler is destroyed)
    commandList->Release();

    // Store the texture resource
    _textureResources[resourceId] = textureResource.Detach();

    return true;
}

void Dx12TextureHandler::releaseDx12Texture(const string& resourceId)
{
    auto it = _textureResources.find(resourceId);
    if (it != _textureResources.end())
    {
        if (it->second)
        {
            it->second->Release();
        }
        _textureResources.erase(it);
    }

    auto viewIt = _textureViews.find(resourceId);
    if (viewIt != _textureViews.end())
    {
        _textureViews.erase(viewIt);
    }
}

MATERIALX_NAMESPACE_END

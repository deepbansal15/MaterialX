//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
#include <MaterialXRenderHlsl/Dx12TextureHandler.h>
#include <MaterialXRender/Renderer.h>
#include <MaterialXRender/Image.h>

#include <d3d12.h>
#include <dxgi1_4.h>

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

    // Get the D3D12 device from the renderer
    // This requires the renderer to provide a way to access the D3D12 device
    // The actual implementation depends on how the D3D12 renderer exposes the device

    // For now, return false as we need the D3D12 device
    // The real implementation would:
    // 1. Get the D3D12 device from the renderer
    // 2. Create a D3D12 resource for the texture
    // 3. Create a shader resource view for the texture
    // 4. Store both in the maps

    return false;
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

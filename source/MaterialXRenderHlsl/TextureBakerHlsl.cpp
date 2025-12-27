//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
#include <MaterialXRenderHlsl/TextureBakerHlsl.h>

#include <MaterialXRender/Direct3D12/Dx12Buffer.h>
#include <MaterialXRender/Direct3D12/Dx12Device.h>
#include <MaterialXRender/Direct3D12/Dx12Texture.h>

MATERIALX_NAMESPACE_BEGIN

TextureBakerHlsl::TextureBakerHlsl(RendererPtr renderer) :
    TextureBaker(renderer)
{
}

TextureBakerHlsl::~TextureBakerHlsl()
{
}

TextureBakerHlslPtr TextureBakerHlsl::create(RendererPtr renderer)
{
    return TextureBakerHlslPtr(new TextureBakerHlsl(renderer));
}

bool TextureBakerHlsl::initialize()
{
    return TextureBaker::initialize();
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
    if (!image)
    {
        return false;
    }

    // Check if we already have a texture with this resource ID
    auto it = _textures.find(resourceId);
    if (it != _textures.end())
    {
        return true; // Already exists
    }

    // Create D3D12 texture
    TexturePtr texture = Dx12Texture::create(resourceId, image, _renderer);
    if (texture)
    {
        _textures[resourceId] = texture;
        return true;
    }

    return false;
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

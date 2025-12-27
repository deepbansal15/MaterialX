//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MATERIALX_DX12TEXTUREHANDLER_H
#define MATERIALX_DX12TEXTUREHANDLER_H

#include <MaterialXRenderHlsl/Export.h>

#include <MaterialXRender/TextureHandler.h>

#include <d3d12.h>

MATERIALX_NAMESPACE_BEGIN

/// @class Dx12TextureHandler
/// A DirectX 12 texture handler implementation for MaterialX
class MX_RENDER_HLSL_API Dx12TextureHandler : public TextureHandler
{
  public:
    /// Create a new Dx12TextureHandler
    static TextureHandlerPtr create(RendererPtr renderer);

    /// Destructor
    ~Dx12TextureHandler() override;

    /// Set the GPU resource for a given resource id
    bool setTexture(const string& resourceId, TexturePtr texture) override;

    /// Get the GPU resource for a given resource id
    ID3D12Resource* getTexture(const string& resourceId) const;

    /// Get the shader resource view for a given resource id
    D3D12_GPU_DESCRIPTOR_HANDLE getTextureView(const string& resourceId) const;

  protected:
    /// Protected constructor
    Dx12TextureHandler(RendererPtr renderer);

    /// Create the GPU resource for a texture
    bool createDx12Texture(const string& resourceId, const ImagePtr image);

    /// Release the GPU resource for a texture
    void releaseDx12Texture(const string& resourceId);

    /// Map from resource id to GPU resource
    std::unordered_map<string, ID3D12Resource*> _textureResources;

    /// Map from resource id to shader resource view
    std::unordered_map<string, D3D12_GPU_DESCRIPTOR_HANDLE> _textureViews;
};

MATERIALX_NAMESPACE_END

#endif

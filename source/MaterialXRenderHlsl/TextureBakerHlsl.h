//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MATERIALX_TEXTUREBAKER_HLSL_H
#define MATERIALX_TEXTUREBAKER_HLSL_H

#include <MaterialXRenderHlsl/Export.h>

#include <MaterialXRender/TextureBaker.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to an HLSL Texture Baker
using TextureBakerHlslPtr = std::shared_ptr<class TextureBakerHlsl>;

/// @class TextureBakerHlsl
/// A texture baker implementation for HLSL/D3D12 platforms.
class MX_RENDER_HLSL_API TextureBakerHlsl : public TextureBaker
{
  public:
    /// Create a new texture baker
    static TextureBakerHlslPtr create(RendererPtr renderer);

    /// Destructor
    ~TextureBakerHlsl() override;

    /// @name Texture Creation Methods
    /// @{

    /// Create a texture from an image file.
    /// @param resourceId Unique identifier for the resource
    /// @param filePath Path to the image file
    /// @param verticalFlip Whether to flip the image vertically
    /// @return True if the texture was created successfully
    bool createTextureFromFile(const string& resourceId, const string& filePath, bool verticalFlip) override;

    /// Create a texture from in-memory image data.
    /// @param resourceId Unique identifier for the resource
    /// @param image The image data
    /// @return True if the texture was created successfully
    bool createTextureFromImage(const string& resourceId, const ImagePtr image) override;

    /// @}

    /// @name Baking Methods
    /// @{

    /// Bake a 2D procedural 2D texture.
    /// @param material The material containing the procedural graph
    /// @param shaderName Name to use for generated shader and texture
    /// @param width Width of the texture to generate
    /// @param height Height of the texture to generate
    /// @param type The output data type
    /// @param filename The output filename for the baked texture
    /// @return True if baking was successful
    bool bake2DTexture(MaterialPtr material,
                       const string& shaderName,
                       unsigned int width,
                       unsigned int height,
                       const string& type,
                       const string& filename) override;

    /// @}

  protected:
    /// Constructor
    TextureBakerHlsl(RendererPtr renderer);

    /// Initialize the baker
    bool initialize() override;

  private:
    /// Helper to create a D3D12 texture resource
    bool createDx12Texture(const string& resourceId, const ImagePtr image);
};

MATERIALX_NAMESPACE_END

#endif // MATERIALX_TEXTUREBAKER_HLSL_H

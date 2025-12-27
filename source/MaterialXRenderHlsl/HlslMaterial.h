//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLMATERIAL_H
#define MATERIALX_HLSLMATERIAL_H

#include <MaterialXRenderHlsl/Export.h>

#include <MaterialXRender/ShaderMaterial.h>
#include <MaterialXRender/Camera.h>
#include <MaterialXRender/LightHandler.h>

#include <MaterialXRenderHlsl/HlslProgram.h>

#include <d3d12.h>
#include <wrl.h>

#include <memory>

MATERIALX_NAMESPACE_BEGIN

using HlslMaterialPtr = shared_ptr<class HlslMaterial>;

/// @class HlslMaterial
/// HLSL implementation of ShaderMaterial for DirectX 12 rendering
class MX_RENDERHLSL_API HlslMaterial : public ShaderMaterial
{
  public:
    /// Create a new HlslMaterial
    static HlslMaterialPtr create();

    /// Destructor
    ~HlslMaterial();

    /// Generate the material shader from a hardware shader
    void generateShader(ShaderPtr hwShader) override;

    /// Bind the shader for rendering
    void bindShader() override;

    /// Unbind the shader
    void unbindShader() override;

    /// Bind mesh data to the shader
    void bindMesh(MeshPtr mesh) override;

    /// Unbind mesh data
    void unbindMesh() override;

    /// Bind view and projection matrices
    void bindViewInformation(CameraPtr camera) override;

    /// Bind lighting state
    void bindLighting(LightHandler* lightHandler, ImageHandlerPtr imageHandler, 
                      const ShadowState& shadowState) override;

    /// Bind images for texture sampling
    void bindImages(ImageHandlerPtr imageHandler, const FileSearchPath& searchPath,
                    bool flipV = false) override;

    /// Unbind images
    void unbindImages(ImageHandlerPtr imageHandler) override;

    /// Draw a mesh partition
    void drawPartition(MeshPartitionPtr partition) override;

    /// Check if material has transparency
    bool hasTransparency() const override { return _hasTransparency; }

    /// Get the shader program
    HlslProgramPtr getProgram() const { return _program; }

  protected:
    /// Constructor
    HlslMaterial();

    /// Create the pipeline state object
    void createPipelineState();

    /// Update uniform values
    void updateUniforms();

    HlslProgramPtr _program;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;

    MeshPtr _boundMesh;
    bool _hasTransparency;

    // Cached uniform values
    Matrix44 _worldMatrix;
    Matrix44 _viewMatrix;
    Matrix44 _projectionMatrix;
    Matrix44 _worldViewProjectionMatrix;
};

MATERIALX_NAMESPACE_END

#endif

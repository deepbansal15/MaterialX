//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslMaterial.h>
#include <MaterialXRenderHlsl/Dx12Renderer.h>
#include <MaterialXRenderHlsl/Dx12TextureHandler.h>

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

HlslMaterial::HlslMaterial() :
    _boundMesh(nullptr),
    _hasTransparency(false)
{
}

HlslMaterial::~HlslMaterial()
{
}

HlslMaterialPtr HlslMaterial::create()
{
    return std::make_shared<HlslMaterial>();
}

void HlslMaterial::generateShader(ShaderPtr hwShader)
{
    if (!hwShader)
    {
        throw std::runtime_error("Invalid shader pointer");
    }

    _shader = hwShader;
    _program = HlslProgram::create();

    // Get vertex and pixel shader source code
    const std::string& vertexSource = hwShader->getSourceCode(Stage::VERTEX);
    const std::string& pixelSource = hwShader->getSourceCode(Stage::PIXEL);

    // Compile shaders
    if (!vertexSource.empty())
    {
        if (!_program->compileVertexShader(vertexSource))
        {
            throw std::runtime_error("Failed to compile vertex shader");
        }
    }

    if (!pixelSource.empty())
    {
        if (!_program->compilePixelShader(pixelSource))
        {
            throw std::runtime_error("Failed to compile pixel shader");
        }
    }

    // Check for transparency
    _hasTransparency = hwShader->hasAttribute(Shader::ATTRIBUTE_TRANSPARENT);
}

void HlslMaterial::bindShader()
{
    if (!_program)
    {
        throw std::runtime_error("No shader program bound");
    }

    // The pipeline state will be created when rendering
    // This method prepares the shader for use
}

void HlslMaterial::unbindShader()
{
    _program.reset();
}

void HlslMaterial::bindMesh(MeshPtr mesh)
{
    _boundMesh = mesh;
}

void HlslMaterial::unbindMesh()
{
    _boundMesh = nullptr;
}

void HlslMaterial::bindViewInformation(CameraPtr camera)
{
    if (!camera)
    {
        throw std::runtime_error("Invalid camera pointer");
    }

    _viewMatrix = camera->getViewMatrix();
    _projectionMatrix = camera->getProjectionMatrix();
    _worldViewProjectionMatrix = _projectionMatrix * _viewMatrix * _worldMatrix;

    // Update uniform values
    updateUniforms();
}

void HlslMaterial::bindLighting(LightHandler* lightHandler, ImageHandlerPtr imageHandler,
                                const ShadowState& shadowState)
{
    if (!lightHandler)
    {
        return;
    }

    // Bind light data uniforms
    // This would be implemented based on the specific lighting model
}

void HlslMaterial::bindImages(ImageHandlerPtr imageHandler, const FileSearchPath& searchPath, bool flipV)
{
    if (!imageHandler)
    {
        return;
    }

    // Bind textures to shader
    // This would be implemented based on the texture binding layout
}

void HlslMaterial::unbindImages(ImageHandlerPtr imageHandler)
{
    // Unbind textures
}

void HlslMaterial::drawPartition(MeshPartitionPtr partition)
{
    if (!_boundMesh || !_program)
    {
        return;
    }

    // Draw the mesh partition
    // The actual rendering would be handled by the render pipeline
}

void HlslMaterial::createPipelineState()
{
    // Create the DirectX 12 pipeline state object
    // This would include:
    // - Root signature
    // - Vertex shader
    // - Pixel shader
    // - Input layout
    // - Rasterizer state
    // - Blend state
    // - Depth stencil state
}

void HlslMaterial::updateUniforms()
{
    // Update uniform values in the shader
    // This would set the world, view, projection matrices and other uniforms
}

MATERIALX_NAMESPACE_END

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

    // Create the pipeline state object if not already created
    if (!_pipelineState)
    {
        createPipelineState();
    }
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

    // Get the Dx12 texture handler if available
    Dx12TextureHandler* dx12Handler = dynamic_cast<Dx12TextureHandler*>(imageHandler.get());
    if (!dx12Handler)
    {
        return;
    }

    // Store the image handler for use during rendering
    // The actual texture binding to descriptor tables would happen in updateUniforms or drawPartition
    // based on the shader's resource binding layout
}

void HlslMaterial::unbindImages(ImageHandlerPtr imageHandler)
{
    // Unbind textures
}

void HlslMaterial::drawPartition(MeshPartitionPtr partition)
{
    if (!_boundMesh || !_program || !_commandList)
    {
        return;
    }

    // Set the pipeline state
    _commandList->SetPipelineState(_pipelineState.Get());

    // Set the root signature
    // Note: In a real implementation, you'd get the root signature from the renderer
    // For now, this is a placeholder

    // Draw the mesh partition
    // The mesh partition contains the geometry data (vertex buffer, index buffer)
    // In a full implementation, you'd:
    // 1. Set vertex and index buffers
    // 2. Set descriptor tables for textures and samplers
    // 3. Draw indexed
}

void HlslMaterial::createPipelineState()
{
    if (!_program)
    {
        return;
    }

    // Get the shader bytecode
    ID3DBlob* vertexShader = _program->getVertexShaderBytecode();
    ID3DBlob* pixelShader = _program->getPixelShaderBytecode();

    if (!vertexShader || !pixelShader)
    {
        return;
    }

    // Define input layout for standard vertex attributes
    // MaterialX uses: POSITION, NORMAL, TEXCOORD, TANGENT
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
    inputLayoutDesc.pInputElementDescs = inputLayout;
    inputLayoutDesc.NumElements = ARRAYSIZE(inputLayout);

    // Describe rasterizer state
    D3D12_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // Describe blend state
    D3D12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    if (_hasTransparency)
    {
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    // Describe depth stencil state
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    if (_hasTransparency)
    {
        depthStencilDesc.DepthEnable = TRUE;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    }

    // Describe the pipeline state
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = nullptr; // Would be set from the renderer
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.BlendState = blendDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.InputLayout = inputLayoutDesc;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;

    // Note: In a real implementation, you'd get the device from the renderer
    // and create the pipeline state object. This is a placeholder structure.
}

void HlslMaterial::updateUniforms()
{
    // Update uniform values in the shader
    // In DirectX 12, uniforms are typically passed through constant buffers
    // This method would:
    // 1. Create or update a constant buffer with the uniform values
    // 2. Set the constant buffer on the root signature

    // Example uniform data that would be uploaded:
    // - World matrix
    // - View matrix
    // - Projection matrix
    // - WorldViewProjection matrix
    // - Camera position
    // - Time values
    // - Any material-specific uniforms

    // The actual implementation depends on the root signature layout
    // and how the shader declares its constant buffers
}

MATERIALX_NAMESPACE_END

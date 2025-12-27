//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/Dx12PipelineStateObject.h>

#include <MaterialXGenShader/Shader.h>

#include <d3dx12.h>

MATERIALX_NAMESPACE_BEGIN

Dx12PipelineStateObject::Dx12PipelineStateObject() :
    _hasTransparency(false)
{
}

Dx12PipelineStateObject::~Dx12PipelineStateObject()
{
}

Dx12PipelineStateObjectPtr Dx12PipelineStateObject::create()
{
    return Dx12PipelineStateObjectPtr(new Dx12PipelineStateObject());
}

bool Dx12PipelineStateObject::initialize(ID3D12Device* device,
                                         ID3D12RootSignature* rootSignature,
                                         ID3DBlob* vertexShader,
                                         ID3DBlob* pixelShader,
                                         const D3D12_INPUT_LAYOUT_DESC& inputLayout,
                                         bool hasTransparency)
{
    if (!device || !rootSignature || !vertexShader || !pixelShader)
    {
        return false;
    }

    _device = device;
    _rootSignature = rootSignature;
    _vertexShader = vertexShader;
    _pixelShader = pixelShader;
    _inputLayout = inputLayout;
    _hasTransparency = hasTransparency;

    return createPipelineState();
}

bool Dx12PipelineStateObject::createPipelineState()
{
    if (!_device || !_rootSignature || !_vertexShader || !_pixelShader)
    {
        return false;
    }

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
    psoDesc.pRootSignature = _rootSignature;
    psoDesc.VS = { _vertexShader->GetBufferPointer(), _vertexShader->GetBufferSize() };
    psoDesc.PS = { _pixelShader->GetBufferPointer(), _pixelShader->GetBufferSize() };
    psoDesc.BlendState = blendDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.InputLayout = _inputLayout;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;

    HRESULT hr = _device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&_pipelineState));
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

void Dx12PipelineStateObject::setHasTransparency(bool hasTransparency)
{
    _hasTransparency = hasTransparency;
}

bool Dx12PipelineStateObject::hasTransparency() const
{
    return _hasTransparency;
}

ID3D12PipelineState* Dx12PipelineStateObject::getPipelineState() const
{
    return _pipelineState.Get();
}

MATERIALX_NAMESPACE_END

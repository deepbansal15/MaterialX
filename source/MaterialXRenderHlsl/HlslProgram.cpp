//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslProgram.h>

#include <MaterialXGenShader/Shader.h>

#include <d3dcompiler.h>

#include <sstream>

MATERIALX_NAMESPACE_BEGIN

HlslProgram::HlslProgram()
{
}

HlslProgram::~HlslProgram()
{
}

HlslProgramPtr HlslProgram::create()
{
    return std::make_shared<HlslProgram>();
}

bool HlslProgram::compileShader(ID3DBlob** shaderBlob, const std::string& source,
                                 const std::string& entryPoint, const std::string& target)
{
    *shaderBlob = nullptr;

    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    compileFlags |= D3DCOMPILE_DEBUG;
    compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    std::wstring wEntryPoint(entryPoint.begin(), entryPoint.end());
    std::wstring wTarget(target.begin(), target.end());

    HRESULT hr = D3DCompile(
        source.c_str(),
        source.size(),
        nullptr,
        nullptr,
        nullptr,
        wEntryPoint.c_str(),
        wTarget.c_str(),
        compileFlags,
        0,
        shaderBlob,
        &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            std::string errorMessage(static_cast<char*>(errorBlob->GetBufferPointer()));
            std::cerr << "HLSL compilation error: " << errorMessage << std::endl;
        }
        return false;
    }

    return true;
}

bool HlslProgram::compileVertexShader(const std::string& source, const std::string& entryPoint)
{
    Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
    if (!compileShader(&shaderBlob, source, entryPoint, "vs_5_0"))
    {
        return false;
    }

    // DirectX 12 uses ID3DBlob for shader bytecode (no separate shader interface types)
    _vertexShader = shaderBlob;
    return true;
}

bool HlslProgram::compilePixelShader(const std::string& source, const std::string& entryPoint)
{
    Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
    if (!compileShader(&shaderBlob, source, entryPoint, "ps_5_0"))
    {
        return false;
    }

    // DirectX 12 uses ID3DBlob for shader bytecode (no separate shader interface types)
    _pixelShader = shaderBlob;
    return true;
}

void HlslProgram::setUniform(const std::string& name, const ValuePtr value)
{
    _uniforms[name] = value;
}

ValuePtr HlslProgram::getUniform(const std::string& name) const
{
    auto it = _uniforms.find(name);
    if (it != _uniforms.end())
    {
        return it->second;
    }
    return nullptr;
}

bool HlslProgram::hasUniform(const std::string& name) const
{
    return _uniforms.find(name) != _uniforms.end();
}

void HlslProgram::clearUniforms()
{
    _uniforms.clear();
}

MATERIALX_NAMESPACE_END

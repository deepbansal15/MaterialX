//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLPROGRAM_H
#define MATERIALX_HLSLPROGRAM_H

#include <MaterialXRenderHlsl/Export.h>

#include <MaterialXCore/Util.h>
#include <MaterialXCore/Value.h>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl.h>

#include <string>
#include <unordered_map>

MATERIALX_NAMESPACE_BEGIN

using HlslProgramPtr = shared_ptr<class HlslProgram>;

/// @class HlslProgram
/// Represents a compiled HLSL shader program
class MX_RENDERHLSL_API HlslProgram
{
  public:
    /// Create a new HlslProgram
    static HlslProgramPtr create();

    /// Destructor
    ~HlslProgram();

    /// Compile a vertex shader from source code
    bool compileVertexShader(const std::string& source, const std::string& entryPoint = "VS_Main");

    /// Compile a pixel shader from source code
    bool compilePixelShader(const std::string& source, const std::string& entryPoint = "PS_Main");

    /// Get the compiled vertex shader bytecode
    ID3DBlob* getVertexShaderBytecode() const { return _vertexShader.Get(); }

    /// Get the vertex shader bytecode size
    size_t getVertexShaderBytecodeSize() const { return _vertexShader ? _vertexShader->GetBufferSize() : 0; }

    /// Get the compiled pixel shader bytecode
    ID3DBlob* getPixelShaderBytecode() const { return _pixelShader.Get(); }

    /// Get the pixel shader bytecode size
    size_t getPixelShaderBytecodeSize() const { return _pixelShader ? _pixelShader->GetBufferSize() : 0; }

    /// Check if the program has a vertex shader
    bool hasVertexShader() const { return _vertexShader != nullptr; }

    /// Check if the program has a pixel shader
    bool hasPixelShader() const { return _pixelShader != nullptr; }

    /// Set a uniform value by name
    void setUniform(const std::string& name, const ValuePtr value);

    /// Get a uniform value by name
    ValuePtr getUniform(const std::string& name) const;

    /// Check if a uniform exists
    bool hasUniform(const std::string& name) const;

    /// Clear all uniforms
    void clearUniforms();

  private:
    /// Constructor
    HlslProgram();

    /// Compile a shader
    bool compileShader(ID3DBlob** shaderBlob, const std::string& source,
                       const std::string& entryPoint, const std::string& target);

    /// Vertex shader bytecode (using ID3DBlob - DirectX 12 doesn't have ID3D12VertexShader)
    Microsoft::WRL::ComPtr<ID3DBlob> _vertexShader;

    /// Pixel shader bytecode (using ID3DBlob - DirectX 12 doesn't have ID3D12PixelShader)
    Microsoft::WRL::ComPtr<ID3DBlob> _pixelShader;

    std::unordered_map<std::string, ValuePtr> _uniforms;
};

MATERIALX_NAMESPACE_END

#endif

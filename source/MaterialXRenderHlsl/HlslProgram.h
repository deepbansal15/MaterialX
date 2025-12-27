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

    /// Get the compiled vertex shader
    ID3D12VertexShader* getVertexShader() const { return _vertexShader.Get(); }

    /// Get the compiled pixel shader
    ID3D12PixelShader* getPixelShader() const { return _pixelShader.Get(); }

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

    Microsoft::WRL::ComPtr<ID3D12VertexShader> _vertexShader;
    Microsoft::WRL::ComPtr<ID3D12PixelShader> _pixelShader;

    std::unordered_map<std::string, ValuePtr> _uniforms;
};

MATERIALX_NAMESPACE_END

#endif

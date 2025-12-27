//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLSHADERGENERATOR_H
#define MATERIALX_HLSLSHADERGENERATOR_H

/// @file
/// HLSL shader generator

#include <MaterialXGenHlsl/Export.h>

#include <MaterialXGenHw/HwResourceBindingContext.h>
#include <MaterialXGenHw/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

using HlslShaderGeneratorPtr = shared_ptr<class HlslShaderGenerator>;

/// @class HlslShaderGenerator
/// An HLSL (High-Level Shading Language) shader generator for Shader Model 5.0
class MX_GENHLSL_API HlslShaderGenerator : public HwShaderGenerator
{
  public:
    /// Constructor.
    HlslShaderGenerator(TypeSystemPtr typeSystem);

    /// Creator function.
    /// If a TypeSystem is not provided it will be created internally.
    /// Optionally pass in an externally created TypeSystem here, 
    /// if you want to keep type descriptions alive after the lifetime
    /// of the shader generator. 
    static ShaderGeneratorPtr create(TypeSystemPtr typeSystem = nullptr)
    {
        return std::make_shared<HlslShaderGenerator>(typeSystem ? typeSystem : TypeSystem::create());
    }

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the HLSL version this generator is for
    const string& getVersion() const { return VERSION; }

    /// Emit a shader variable.
    void emitVariableDeclaration(const ShaderPort* variable, const string& qualifier, GenContext& context, ShaderStage& stage,
                                 bool assignValue = true) const override;

    /// Determine the prefix of vertex data variables.
    string getVertexDataPrefix(const VariableBlock& vertexData) const override;

  public:
    /// Unique identifier for this generator target
    static const string TARGET;

    /// Version string for the generator target (HLSL Shader Model 5.0)
    static const string VERSION;

  protected:
    void emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;
    void emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    void emitDirectives(GenContext& context, ShaderStage& stage) const;
    void emitConstants(GenContext& context, ShaderStage& stage) const;
    void emitUniforms(GenContext& context, ShaderStage& stage) const;
    void emitLightData(GenContext& context, ShaderStage& stage) const;
    void emitInputs(GenContext& context, ShaderStage& stage) const;
    void emitOutputs(GenContext& context, ShaderStage& stage) const;

    HwResourceBindingContextPtr getResourceBindingContext(GenContext& context) const;

    /// Emit specular environment lookup code
    void emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const;

    /// Emit transmission rendering code
    void emitTransmissionRender(GenContext& context, ShaderStage& stage) const;

    /// Emit function definitions for lighting code
    void emitLightFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    /// Nodes used internally for light sampling.
    vector<ShaderNodePtr> _lightSamplingNodes;

    /// Binding location counter for samplers
    mutable size_t _hwSamplerBindLocation = 0;
};

MATERIALX_NAMESPACE_END

#endif

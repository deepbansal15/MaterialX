//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHlsl/HlslResourceBindingContext.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

//
// HlslResourceBindingContext
//
HlslResourceBindingContext::HlslResourceBindingContext(
    size_t uniformBindingLocation, size_t samplerBindingLocation) :
    _hwInitUniformBindLocation(uniformBindingLocation),
    _hwInitSamplerBindLocation(samplerBindingLocation)
{
    // Initialize runtime binding locations to match initial values
    _hwUniformBindLocation = _hwInitUniformBindLocation;
    _hwSamplerBindLocation = _hwInitSamplerBindLocation;
}

void HlslResourceBindingContext::initialize()
{
    // Reset bind location counter for uniforms.
    _hwUniformBindLocation = _hwInitUniformBindLocation;

    // Reset bind location counter for samplers.
    _hwSamplerBindLocation = _hwInitSamplerBindLocation;
}

void HlslResourceBindingContext::emitDirectives(GenContext&, ShaderStage& stage)
{
    // HLSL SM 5.0 doesn't require any special directives for resource binding
    // The profile is specified during compilation, not in the shader code
}

void HlslResourceBindingContext::emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage)
{
    const ShaderGenerator& generator = context.getShaderGenerator();
    const Syntax& syntax = generator.getSyntax();

    // First, emit all value uniforms in a cbuffer with register binding
    bool hasValueUniforms = false;
    for (auto uniform : uniforms.getVariableOrder())
    {
        if (uniform->getType() != Type::FILENAME)
        {
            hasValueUniforms = true;
            break;
        }
    }
    if (hasValueUniforms)
    {
        generator.emitLine("cbuffer " + uniforms.getName() + "_" + stage.getName() + " : register(b" + std::to_string(_hwUniformBindLocation++) + ")",
                           stage, false);
        generator.emitScopeBegin(stage);
        for (auto uniform : uniforms.getVariableOrder())
        {
            if (uniform->getType() != Type::FILENAME)
            {
                generator.emitLineBegin(stage);
                generator.emitVariableDeclaration(uniform, EMPTY_STRING, context, stage, false);
                generator.emitString(Syntax::SEMICOLON, stage);
                generator.emitLineEnd(stage, false);
            }
        }
        generator.emitScopeEnd(stage, true);
    }

    // Second, emit all sampler uniforms as separate Texture2D and SamplerState with register bindings
    for (auto uniform : uniforms.getVariableOrder())
    {
        if (uniform->getType() == Type::FILENAME)
        {
            // Emit Texture2D with register binding
            generator.emitString(syntax.getUniformQualifier() + " " + syntax.getTypeName(uniform->getType()) + " " + uniform->getVariable(), stage);
            generator.emitString(" : register(t" + std::to_string(_hwUniformBindLocation++) + ")", stage);
            generator.emitLineEnd(stage, true);

            // Emit SamplerState with register binding
            generator.emitString(syntax.getUniformQualifier() + " SamplerState " + uniform->getVariable() + "_sampler", stage);
            generator.emitString(" : register(s" + std::to_string(_hwSamplerBindLocation++) + ")", stage);
            generator.emitLineEnd(stage, true);
        }
    }

    generator.emitLineBreak(stage);
}

void HlslResourceBindingContext::emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
                                                                ShaderStage& stage, const std::string& structInstanceName,
                                                                const std::string& arraySuffix)
{
    const ShaderGenerator& generator = context.getShaderGenerator();
    const Syntax& syntax = generator.getSyntax();

    // HLSL structures don't require manual alignment padding like GLSL std140 layout.
    // The HLSL compiler handles alignment automatically according to HLSL packing rules.
    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules

    // Emit the struct definition
    generator.emitLine("struct " + uniforms.getName(), stage, false);
    generator.emitScopeBegin(stage);

    for (size_t i = 0; i < uniforms.size(); ++i)
    {
        generator.emitLineBegin(stage);
        generator.emitVariableDeclaration(
            uniforms[i], EMPTY_STRING, context, stage, false);
        generator.emitString(Syntax::SEMICOLON, stage);
        generator.emitLineEnd(stage, false);
    }

    generator.emitScopeEnd(stage, true);

    // Emit binding information
    generator.emitLineBreak(stage);
    generator.emitLine("cbuffer " + uniforms.getName() + "_" + stage.getName() + " : register(b" + std::to_string(_hwUniformBindLocation++) + ")",
                       stage, false);
    generator.emitScopeBegin(stage);
    generator.emitLine(uniforms.getName() + " " + structInstanceName + arraySuffix, stage);
    generator.emitScopeEnd(stage, true);
}

MATERIALX_NAMESPACE_END

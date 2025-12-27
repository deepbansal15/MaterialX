//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLRESOURCEBINDINGCONTEXT_H
#define MATERIALX_HLSLRESOURCEBINDINGCONTEXT_H

/// @file
/// HLSL resource binding context

#include <MaterialXGenHlsl/Export.h>

#include <MaterialXGenHw/HwResourceBindingContext.h>

MATERIALX_NAMESPACE_BEGIN

/// Shared pointer to a HlslResourceBindingContext
using HlslResourceBindingContextPtr = shared_ptr<class HlslResourceBindingContext>;

/// @class HlslResourceBindingContext
/// Class representing a resource binding for HLSL shader resources.
class MX_GENHLSL_API HlslResourceBindingContext : public HwResourceBindingContext
{
  public:
    HlslResourceBindingContext(size_t uniformBindingLocation = 0, size_t samplerBindingLocation = 0);

    static HlslResourceBindingContextPtr create(size_t uniformBindingLocation = 0, size_t samplerBindingLocation = 0)
    {
        return std::make_shared<HlslResourceBindingContext>(uniformBindingLocation, samplerBindingLocation);
    }

    // Initialize the context before generation starts.
    void initialize() override;

    // Emit directives for stage
    void emitDirectives(GenContext& context, ShaderStage& stage) override;

    // Emit uniforms with binding information
    void emitResourceBindings(GenContext& context, const VariableBlock& uniforms, ShaderStage& stage) override;

    // Emit structured uniforms with binding information and align members where possible
    void emitStructuredResourceBindings(GenContext& context, const VariableBlock& uniforms,
                                        ShaderStage& stage, const std::string& structInstanceName,
                                        const std::string& arraySuffix) override;

  protected:
    // Binding location for uniform blocks
    size_t _hwUniformBindLocation = 0;
    size_t _hwInitUniformBindLocation = 0;

    // Binding location for sampler blocks
    size_t _hwSamplerBindLocation = 0;
    size_t _hwInitSamplerBindLocation = 0;
};

MATERIALX_NAMESPACE_END

#endif

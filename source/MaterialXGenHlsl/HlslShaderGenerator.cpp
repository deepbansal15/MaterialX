//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXGenHlsl/HlslSyntax.h>

#include <MaterialXGenHw/HwLightShaders.h>
#include <MaterialXGenHw/Nodes/HwImageNode.h>
#include <MaterialXGenHw/Nodes/HwGeomColorNode.h>
#include <MaterialXGenHw/Nodes/HwGeomPropValueNode.h>
#include <MaterialXGenHw/Nodes/HwTexCoordNode.h>
#include <MaterialXGenHw/Nodes/HwTransformNode.h>
#include <MaterialXGenHw/Nodes/HwPositionNode.h>
#include <MaterialXGenHw/Nodes/HwNormalNode.h>
#include <MaterialXGenHw/Nodes/HwTangentNode.h>
#include <MaterialXGenHw/Nodes/HwBitangentNode.h>
#include <MaterialXGenHw/Nodes/HwFrameNode.h>
#include <MaterialXGenHw/Nodes/HwTimeNode.h>
#include <MaterialXGenHw/Nodes/HwViewDirectionNode.h>
#include <MaterialXGenHw/Nodes/HwLightCompoundNode.h>
#include <MaterialXGenHw/Nodes/HwLightNode.h>
#include <MaterialXGenHw/Nodes/HwLightSamplerNode.h>
#include <MaterialXGenHw/Nodes/HwLightShaderNode.h>
#include <MaterialXGenHw/Nodes/HwNumLightsNode.h>
#include <MaterialXGenHw/Nodes/HwSurfaceNode.h>

#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Nodes/MaterialNode.h>

MATERIALX_NAMESPACE_BEGIN

const string HlslShaderGenerator::TARGET = "genhlsl";
const string HlslShaderGenerator::VERSION = "5_0";

HlslShaderGenerator::HlslShaderGenerator(TypeSystemPtr typeSystem) :
    HwShaderGenerator(typeSystem, HlslSyntax::create(typeSystem))
{
    //
    // Register all custom node implementation classes
    //

    StringVec elementNames;

    // <!-- <position> -->
    registerImplementation("IM_position_vector3_" + HlslShaderGenerator::TARGET, HwPositionNode::create);
    // <!-- <normal> -->
    registerImplementation("IM_normal_vector3_" + HlslShaderGenerator::TARGET, HwNormalNode::create);
    // <!-- <tangent> -->
    registerImplementation("IM_tangent_vector3_" + HlslShaderGenerator::TARGET, HwTangentNode::create);
    // <!-- <bitangent> -->
    registerImplementation("IM_bitangent_vector3_" + HlslShaderGenerator::TARGET, HwBitangentNode::create);
    // <!-- <texcoord> -->
    registerImplementation("IM_texcoord_vector2_" + HlslShaderGenerator::TARGET, HwTexCoordNode::create);
    registerImplementation("IM_texcoord_vector3_" + HlslShaderGenerator::TARGET, HwTexCoordNode::create);
    // <!-- <geomcolor> -->
    registerImplementation("IM_geomcolor_float_" + HlslShaderGenerator::TARGET, HwGeomColorNode::create);
    registerImplementation("IM_geomcolor_color3_" + HlslShaderGenerator::TARGET, HwGeomColorNode::create);
    registerImplementation("IM_geomcolor_color4_" + HlslShaderGenerator::TARGET, HwGeomColorNode::create);
    // <!-- <geompropvalue> -->
    elementNames = {
        "IM_geompropvalue_integer_" + HlslShaderGenerator::TARGET,
        "IM_geompropvalue_float_" + HlslShaderGenerator::TARGET,
        "IM_geompropvalue_color3_" + HlslShaderGenerator::TARGET,
        "IM_geompropvalue_color4_" + HlslShaderGenerator::TARGET,
        "IM_geompropvalue_vector2_" + HlslShaderGenerator::TARGET,
        "IM_geompropvalue_vector3_" + HlslShaderGenerator::TARGET,
        "IM_geompropvalue_vector4_" + HlslShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, HwGeomPropValueNode::create);
    registerImplementation("IM_geompropvalue_boolean_" + HlslShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);
    registerImplementation("IM_geompropvalue_string_" + HlslShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);
    registerImplementation("IM_geompropvalue_filename_" + HlslShaderGenerator::TARGET, HwGeomPropValueNodeAsUniform::create);

    // <!-- <frame> -->
    registerImplementation("IM_frame_float_" + HlslShaderGenerator::TARGET, HwFrameNode::create);
    // <!-- <time> -->
    registerImplementation("IM_time_float_" + HlslShaderGenerator::TARGET, HwTimeNode::create);
    // <!-- <viewdirection> -->
    registerImplementation("IM_viewdirection_vector3_" + HlslShaderGenerator::TARGET, HwViewDirectionNode::create);

    // <!-- <surface> -->
    registerImplementation("IM_surface_" + HlslShaderGenerator::TARGET, HwSurfaceNode::create);

    // <!-- <light> -->
    registerImplementation("IM_light_" + HlslShaderGenerator::TARGET, HwLightNode::create);

    // <!-- <point_light> -->
    registerImplementation("IM_point_light_" + HlslShaderGenerator::TARGET, HwLightShaderNode::create);
    // <!-- <directional_light> -->
    registerImplementation("IM_directional_light_" + HlslShaderGenerator::TARGET, HwLightShaderNode::create);
    // <!-- <spot_light> -->
    registerImplementation("IM_spot_light_" + HlslShaderGenerator::TARGET, HwLightShaderNode::create);

    // <!-- <ND_transformpoint> ->
    registerImplementation("IM_transformpoint_vector3_" + HlslShaderGenerator::TARGET, HwTransformPointNode::create);

    // <!-- <ND_transformvector> ->
    registerImplementation("IM_transformvector_vector3_" + HlslShaderGenerator::TARGET, HwTransformVectorNode::create);

    // <!-- <ND_transformnormal> ->
    registerImplementation("IM_transformnormal_vector3_" + HlslShaderGenerator::TARGET, HwTransformNormalNode::create);

    // <!-- <image> -->
    elementNames = {
        "IM_image_float_" + HlslShaderGenerator::TARGET,
        "IM_image_color3_" + HlslShaderGenerator::TARGET,
        "IM_image_color4_" + HlslShaderGenerator::TARGET,
        "IM_image_vector2_" + HlslShaderGenerator::TARGET,
        "IM_image_vector3_" + HlslShaderGenerator::TARGET,
        "IM_image_vector4_" + HlslShaderGenerator::TARGET,
    };
    registerImplementation(elementNames, HwImageNode::create);

    // <!-- <surfacematerial> -->
    registerImplementation("IM_surfacematerial_" + HlslShaderGenerator::TARGET, MaterialNode::create);

    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "numActiveLightSources", HwNumLightsNode::create()));
    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "sampleLightSource", HwLightSamplerNode::create()));
}

ShaderPtr HlslShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    ShaderPtr shader = createShader(name, element, context);

    // Request fixed floating-point notation for consistency across targets.
    ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    // Make sure we initialize/reset the binding context before generation.
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->initialize();
    }

    // Emit code for vertex shader stage
    ShaderStage& vs = shader->getStage(Stage::VERTEX);
    emitVertexStage(shader->getGraph(), context, vs);
    replaceTokens(_tokenSubstitutions, vs);

    // Emit code for pixel shader stage
    ShaderStage& ps = shader->getStage(Stage::PIXEL);
    emitPixelStage(shader->getGraph(), context, ps);
    replaceTokens(_tokenSubstitutions, ps);

    return shader;
}

void HlslShaderGenerator::emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);

    emitDirectives(context, stage);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->emitDirectives(context, stage);
    }
    emitLineBreak(stage);

    // Add all constants
    emitConstants(context, stage);

    // Add all uniforms
    emitUniforms(context, stage);

    // Add vertex inputs
    emitInputs(context, stage);

    // Add vertex data outputs block
    emitOutputs(context, stage);

    // Add common math functions
    emitLibraryInclude("stdlib/genhlsl/lib/mx_math.hlsl", context, stage);
    emitLineBreak(stage);

    emitFunctionDefinitions(graph, context, stage);

    // Add main function
    setFunctionName("VS_Main", stage);
    emitLine("VSOutput VS_Main(VSInput input)", stage, false);
    emitFunctionBodyBegin(graph, context, stage);
    emitLine("VSOutput output;", stage);
    emitLine("float4 hPositionWorld = mul(float4(input.position, 1.0), worldMatrix);", stage);
    emitLine("output.position = mul(hPositionWorld, viewProjectionMatrix);", stage);

    // Emit all function calls in order
    for (const ShaderNode* node : graph.getNodes())
    {
        emitFunctionCall(*node, context, stage);
    }

    emitFunctionBodyEnd(graph, context, stage);
}

void HlslShaderGenerator::emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const
{
    int specularMethod = context.getOptions().hwSpecularEnvironmentMethod;
    if (specularMethod == SPECULAR_ENVIRONMENT_FIS)
    {
        emitLibraryInclude("pbrlib/genhlsl/lib/mx_environment_fis.hlsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_PREFILTER)
    {
        emitLibraryInclude("pbrlib/genhlsl/lib/mx_environment_prefilter.hlsl", context, stage);
    }
    else if (specularMethod == SPECULAR_ENVIRONMENT_NONE)
    {
        emitLibraryInclude("pbrlib/genhlsl/lib/mx_environment_none.hlsl", context, stage);
    }
    else
    {
        throw ExceptionShaderGenError("Invalid hardware specular environment method specified: '" + std::to_string(specularMethod) + "'");
    }
    emitLineBreak(stage);
}

void HlslShaderGenerator::emitTransmissionRender(GenContext& context, ShaderStage& stage) const
{
    int transmissionMethod = context.getOptions().hwTransmissionRenderMethod;
    if (transmissionMethod == TRANSMISSION_REFRACTION)
    {
        emitLibraryInclude("pbrlib/genhlsl/lib/mx_transmission_refract.hlsl", context, stage);
    }
    else if (transmissionMethod == TRANSMISSION_OPACITY)
    {
        emitLibraryInclude("pbrlib/genhlsl/lib/mx_transmission_opacity.hlsl", context, stage);
    }
    else
    {
        throw ExceptionShaderGenError("Invalid transmission render specified: '" + std::to_string(transmissionMethod) + "'");
    }
    emitLineBreak(stage);
}

void HlslShaderGenerator::emitDirectives(GenContext&, ShaderStage& stage) const
{
    // HLSL SM 5.0 doesn't require version directives
    // The profile is specified during compilation
}

void HlslShaderGenerator::emitConstants(GenContext& context, ShaderStage& stage) const
{
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), Syntax::SEMICOLON, context, stage);
        emitLineBreak(stage);
    }
}

void HlslShaderGenerator::emitUniforms(GenContext& context, ShaderStage& stage) const
{
    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;

        // Skip light uniforms as they are handled separately
        if (!uniforms.empty() && uniforms.getName() != HW::LIGHT_DATA)
        {
            emitComment("Uniform block: " + uniforms.getName(), stage);
            HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
            if (resourceBindingCtx)
            {
                resourceBindingCtx->emitResourceBindings(context, uniforms, stage);
            }
            else
            {
                emitVariableDeclarations(uniforms, _syntax->getUniformQualifier(), Syntax::SEMICOLON, context, stage);
                emitLineBreak(stage);
            }
        }
    }
}

void HlslShaderGenerator::emitLightData(GenContext& context, ShaderStage& stage) const
{
    const VariableBlock& lightData = stage.getUniformBlock(HW::LIGHT_DATA);
    const string structArraySuffix = "[" + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + "]";
    const string structName = lightData.getInstance();
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->emitStructuredResourceBindings(
            context, lightData, stage, structName, structArraySuffix);
    }
    else
    {
        emitLine("struct " + lightData.getName(), stage, false);
        emitScopeBegin(stage);
        emitVariableDeclarations(lightData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
        emitScopeEnd(stage, true);
        emitLineBreak(stage);
        emitLine(structName + structArraySuffix, stage);
    }
    emitLineBreak(stage);
}

void HlslShaderGenerator::emitInputs(GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
        if (!vertexInputs.empty())
        {
            emitComment("Inputs block: " + vertexInputs.getName(), stage);
            
            // Emit vertex input structure
            emitLine("struct VSInput", stage, false);
            emitScopeBegin(stage);
            for (size_t i = 0; i < vertexInputs.size(); ++i)
            {
                const ShaderPort* port = vertexInputs[i];
                string semantic = "";
                if (port->getName() == HW::T_IN_POSITION)
                    semantic = " : POSITION";
                else if (port->getName() == HW::T_IN_NORMAL)
                    semantic = " : NORMAL";
                else if (port->getName() == HW::T_IN_TANGENT)
                    semantic = " : TANGENT";
                else if (port->getName() == HW::T_IN_TEXCOORD)
                    semantic = " : TEXCOORD0";
                
                emitLine(_syntax->getTypeName(port->getType()) + " " + port->getVariable() + semantic, stage);
            }
            emitScopeEnd(stage, true);
            emitLineBreak(stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        if (!vertexData.empty())
        {
            emitLine("struct VSOutput", stage, false);
            emitScopeBegin(stage);
            emitLine("float4 position : SV_Position;", stage);
            emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
            emitScopeEnd(stage, false, false);
            emitLineBreak(stage);
        }
    }
}

void HlslShaderGenerator::emitOutputs(GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::VERTEX)
    {
        const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        if (!vertexData.empty())
        {
            emitLine("struct VSOutput", stage, false);
            emitScopeBegin(stage);
            emitLine("float4 position : SV_Position;", stage);
            emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
            emitScopeEnd(stage, false, false);
            emitLineBreak(stage);
        }
    }

    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        emitComment("Pixel shader outputs", stage);
        const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
        
        // Emit pixel output structure
        emitLine("struct PSOutput", stage, false);
        emitScopeBegin(stage);
        emitLine("float4 color : SV_Target0;", stage);
        emitVariableDeclarations(outputs, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
        emitScopeEnd(stage, true);
        emitLineBreak(stage);
    }
}

HwResourceBindingContextPtr HlslShaderGenerator::getResourceBindingContext(GenContext& context) const
{
    return context.getUserData<HwResourceBindingContext>(HW::USER_DATA_BINDING_CONTEXT);
}

string HlslShaderGenerator::getVertexDataPrefix(const VariableBlock& vertexData) const
{
    return "input.";
}

void HlslShaderGenerator::emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);

    // Add directives
    emitDirectives(context, stage);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->emitDirectives(context, stage);
    }
    emitLineBreak(stage);

    // Add type definitions
    emitTypeDefinitions(context, stage);

    // Add all constants
    emitConstants(context, stage);

    // Add all uniforms
    emitUniforms(context, stage);

    // Add vertex data inputs block
    emitInputs(context, stage);

    // Add the pixel shader output.
    emitOutputs(context, stage);

    // Add common math functions
    emitLibraryInclude("stdlib/genhlsl/lib/mx_math.hlsl", context, stage);
    emitLineBreak(stage);

    // Determine whether lighting is required
    bool lighting = requiresLighting(graph);

    // Define directional albedo approach
    if (lighting || context.getOptions().hwWriteAlbedoTable || context.getOptions().hwWriteEnvPrefilter)
    {
        emitLine("#define DIRECTIONAL_ALBEDO_METHOD " + std::to_string(int(context.getOptions().hwDirectionalAlbedoMethod)), stage, false);
        emitLineBreak(stage);
    }

    // Define Airy Fresnel iterations
    emitLine("#define AIRY_FRESNEL_ITERATIONS " + std::to_string(context.getOptions().hwAiryFresnelIterations), stage, false);
    emitLineBreak(stage);

    // Add lighting support
    if (lighting)
    {
        if (context.getOptions().hwMaxActiveLightSources > 0)
        {
            const unsigned int maxLights = std::max(1u, context.getOptions().hwMaxActiveLightSources);
            emitLine("#define " + HW::LIGHT_DATA_MAX_LIGHT_SOURCES + " " + std::to_string(maxLights), stage, false);
        }
        emitSpecularEnvironment(context, stage);
        emitTransmissionRender(context, stage);

        if (context.getOptions().hwMaxActiveLightSources > 0)
        {
            emitLightData(context, stage);
        }
    }

    // Add shadowing support
    bool shadowing = (lighting && context.getOptions().hwShadowMap) ||
                     context.getOptions().hwWriteDepthMoments;
    if (shadowing)
    {
        emitLibraryInclude("pbrlib/genhlsl/lib/mx_shadow.hlsl", context, stage);
        emitLibraryInclude("pbrlib/genhlsl/lib/mx_shadow_platform.hlsl", context, stage);
    }

    // Emit directional albedo table code.
    if (context.getOptions().hwWriteAlbedoTable)
    {
        emitLibraryInclude("pbrlib/genhlsl/lib/mx_generate_albedo_table.hlsl", context, stage);
        emitLineBreak(stage);
    }

    // Emit environment prefiltering code
    if (context.getOptions().hwWriteEnvPrefilter)
    {
        emitLibraryInclude("pbrlib/genhlsl/lib/mx_generate_prefilter_env.hlsl", context, stage);
        emitLineBreak(stage);
    }

    // Set the include file to use for uv transformations,
    // depending on the vertical flip flag.
    if (context.getOptions().fileTextureVerticalFlip)
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "mx_transform_uv_vflip.hlsl";
    }
    else
    {
        _tokenSubstitutions[ShaderGenerator::T_FILE_TRANSFORM_UV] = "mx_transform_uv.hlsl";
    }

    emitLightFunctionDefinitions(graph, context, stage);

    // Emit function definitions for all nodes in the graph.
    emitFunctionDefinitions(graph, context, stage);

    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket();

    // Add main function
    setFunctionName("PS_Main", stage);
    emitLine("PSOutput PS_Main(VSOutput input) : SV_Target0", stage, false);
    emitFunctionBodyBegin(graph, context, stage);

    if (graph.hasClassification(ShaderNode::Classification::CLOSURE) &&
        !graph.hasClassification(ShaderNode::Classification::SHADER))
    {
        // Handle the case where the graph is a direct closure.
        // We don't support rendering closures without attaching
        // to a surface shader, so just output black.
        emitLine("PSOutput output;", stage);
        emitLine("output.color = float4(0.0, 0.0, 0.0, 1.0);", stage);
    }
    else if (context.getOptions().hwWriteDepthMoments)
    {
        emitLine("PSOutput output;", stage);
        emitLine("float depthMoments = mx_compute_depth_moments();", stage);
        emitLine("output.color = float4(depthMoments, 0.0, 0.0, 1.0);", stage);
    }
    else if (context.getOptions().hwWriteAlbedoTable)
    {
        emitLine("PSOutput output;", stage);
        emitLine("float3 dirAlbedo = mx_generate_dir_albedo_table();", stage);
        emitLine("output.color = float4(dirAlbedo, 1.0);", stage);
    }
    else if (context.getOptions().hwWriteEnvPrefilter)
    {
        emitLine("PSOutput output;", stage);
        emitLine("float4 prefilterEnv = mx_generate_prefilter_env();", stage);
        emitLine("output.color = prefilterEnv;", stage);
    }
    else
    {
        // Add all function calls.
        //
        // Surface shaders need special handling.
        if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
        {
            // Emit all texturing nodes. These are inputs to any
            // closure/shader nodes and need to be emitted first.
            emitFunctionCalls(graph, context, stage, ShaderNode::Classification::TEXTURE);

            // Emit function calls for "root" closure/shader nodes.
            // These will internally emit function calls for any dependent closure nodes upstream.
            for (ShaderGraphOutputSocket* socket : graph.getOutputSockets())
            {
                if (socket->getConnection())
                {
                    const ShaderNode* upstream = socket->getConnection()->getNode();
                    if (upstream->getParent() == &graph &&
                        (upstream->hasClassification(ShaderNode::Classification::CLOSURE) ||
                         upstream->hasClassification(ShaderNode::Classification::SHADER)))
                    {
                        emitFunctionCall(*upstream, context, stage);
                    }
                }
            }
        }
        else
        {
            // No surface shader graph so just generate all
            // function calls in order.
            emitFunctionCalls(graph, context, stage);
        }

        // Emit final output
        const ShaderOutput* outputConnection = outputSocket->getConnection();
        if (outputConnection)
        {
            if (graph.hasClassification(ShaderNode::Classification::SURFACE))
            {
                string outColor = outputConnection->getVariable() + ".color";
                string outTransparency = outputConnection->getVariable() + ".transparency";
                if (context.getOptions().hwSrgbEncodeOutput)
                {
                    outColor = "mx_srgb_encode(" + outColor + ")";
                }
                emitLine("PSOutput output;", stage);
                if (context.getOptions().hwTransparency)
                {
                    emitLine("float outAlpha = saturate(1.0 - dot(" + outTransparency + ", float3(0.3333, 0.3334, 0.3333)));", stage);
                    emitLine("output.color = float4(" + outColor + ", outAlpha);", stage);
                    emitLine("if (outAlpha < " + HW::T_ALPHA_THRESHOLD + ")", stage, false);
                    emitScopeBegin(stage);
                    emitLine("discard", stage);
                    emitScopeEnd(stage);
                }
                else
                {
                    emitLine("output.color = float4(" + outColor + ", 1.0);", stage);
                }
            }
            else
            {
                string outValue = outputConnection->getVariable();
                if (context.getOptions().hwSrgbEncodeOutput && outputSocket->getType().isFloat3())
                {
                    outValue = "mx_srgb_encode(" + outValue + ")";
                }
                if (!outputSocket->getType().isFloat4())
                {
                    toVec4(outputSocket->getType(), outValue);
                }
                emitLine("PSOutput output;", stage);
                emitLine("output.color = " + outValue + ";", stage);
            }
        }
        else
        {
            string outputValue = outputSocket->getValue() ?
                                 _syntax->getValue(outputSocket->getType(), *outputSocket->getValue()) :
                                 _syntax->getDefaultValue(outputSocket->getType());
            emitLine("PSOutput output;", stage);
            if (!outputSocket->getType().isFloat4())
            {
                string finalOutput = outputSocket->getVariable() + "_tmp";
                emitLine(_syntax->getTypeName(outputSocket->getType()) + " " + finalOutput + " = " + outputValue + ";", stage);
                toVec4(outputSocket->getType(), finalOutput);
                emitLine("output.color = " + finalOutput + ";", stage);
            }
            else
            {
                emitLine("output.color = " + outputValue + ";", stage);
            }
        }
    }

    // End main function
    emitFunctionBodyEnd(graph, context, stage);
}

void HlslShaderGenerator::emitLightFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        // Emit Light functions if requested
        if (requiresLighting(graph) && context.getOptions().hwMaxActiveLightSources > 0)
        {
            // For surface shaders we need light shaders
            if (graph.hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
            {
                // Emit functions for all bound light shaders
                HwLightShadersPtr lightShaders = context.getUserData<HwLightShaders>(HW::USER_DATA_LIGHT_SHADERS);
                if (lightShaders)
                {
                    for (const auto& it : lightShaders->get())
                    {
                        emitFunctionDefinition(*it.second, context, stage);
                    }
                }
                // Emit functions for light sampling
                for (const auto& it : _lightSamplingNodes)
                {
                    emitFunctionDefinition(*it, context, stage);
                }
            }
        }
    }
}

void HlslShaderGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                                  GenContext& context, ShaderStage& stage,
                                                  bool assignValue) const
{
    // A file texture input needs special handling on HLSL
    if (variable->getType() == Type::FILENAME)
    {
        // Get the resource binding context for sampler binding locations
        HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
        
        // For textures, we declare both Texture2D and SamplerState
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        emitString(str + "Texture2D " + variable->getVariable(), stage);
        
        // Use resource binding context for register binding if available
        // Otherwise, use a simple default binding (t0, s0)
        if (resourceBindingCtx)
        {
            // The binding context will handle register assignments through emitResourceBindings
            emitLineEnd(stage, true);
        }
        else
        {
            // Fallback: no register binding (binding will be set at shader creation time)
            emitLineEnd(stage, true);
        }
        
        emitString(str + "SamplerState " + variable->getVariable() + "_sampler", stage);
        emitLineEnd(stage, true);
    }
    else
    {
        string str = qualifier.empty() ? EMPTY_STRING : qualifier + " ";
        str += _syntax->getTypeName(variable->getType()) + " " + variable->getVariable();

        // If an array we need an array qualifier (suffix) for the variable name
        if (variable->getType().isArray() && variable->getValue())
        {
            str += _syntax->getArrayVariableSuffix(variable->getType(), *variable->getValue());
        }

        if (!variable->getSemantic().empty())
        {
            str += " : " + variable->getSemantic();
        }

        if (assignValue)
        {
            const string valueStr = (variable->getValue() ?
                                    _syntax->getValue(variable->getType(), *variable->getValue(), true) :
                                    _syntax->getDefaultValue(variable->getType(), true));
            str += valueStr.empty() ? EMPTY_STRING : " = " + valueStr;
        }

        emitString(str, stage);
    }
}

MATERIALX_NAMESPACE_END

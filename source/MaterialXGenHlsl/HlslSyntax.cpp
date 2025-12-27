//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHlsl/HlslSyntax.h>

#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <memory>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Since HLSL doesn't support strings we use integers instead.
// TODO: Support options strings by converting to a corresponding enum integer
class HlslStringTypeSyntax : public StringTypeSyntax
{
  public:
    HlslStringTypeSyntax(const Syntax* parent) :
        StringTypeSyntax(parent, "int", "0", "0") { }

    string getValue(const Value& /*value*/, bool /*uniform*/) const override
    {
        return "0";
    }
};

class HlslArrayTypeSyntax : public ScalarTypeSyntax
{
  public:
    HlslArrayTypeSyntax(const Syntax* parent, const string& name) :
        ScalarTypeSyntax(parent, name, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING)
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        size_t arraySize = getSize(value);
        if (arraySize > 0)
        {
            return _name + "[" + std::to_string(arraySize) + "](" + value.getValueString() + ")";
        }
        return EMPTY_STRING;
    }

  protected:
    virtual size_t getSize(const Value& value) const = 0;
};

class HlslFloatArrayTypeSyntax : public HlslArrayTypeSyntax
{
  public:
    explicit HlslFloatArrayTypeSyntax(const Syntax* parent, const string& name) :
        HlslArrayTypeSyntax(parent, name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<float> valueArray = value.asA<vector<float>>();
        return valueArray.size();
    }
};

class HlslIntegerArrayTypeSyntax : public HlslArrayTypeSyntax
{
  public:
    explicit HlslIntegerArrayTypeSyntax(const Syntax* parent, const string& name) :
        HlslArrayTypeSyntax(parent, name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<int> valueArray = value.asA<vector<int>>();
        return valueArray.size();
    }
};

} // anonymous namespace

const string HlslSyntax::CONSTANT_QUALIFIER = "static const";
const string HlslSyntax::UNIFORM_QUALIFIER = "uniform";
const string HlslSyntax::SOURCE_FILE_EXTENSION = ".hlsl";
const StringVec HlslSyntax::VEC2_MEMBERS = { ".x", ".y" };
const StringVec HlslSyntax::VEC3_MEMBERS = { ".x", ".y", ".z" };
const StringVec HlslSyntax::VEC4_MEMBERS = { ".x", ".y", ".z", ".w" };

//
// HlslSyntax methods
//

HlslSyntax::HlslSyntax(TypeSystemPtr typeSystem) :
    Syntax(typeSystem)
{
    // Add in all reserved words and keywords in HLSL
    registerReservedWords(
        { "break", "case", "continue", "default", "do", "else", "for", "if", "return", "switch", "while",
          "bool", "double", "false", "float", "half", "int", "long", "short", "true", "unsigned", "void",
          "char", "signed", "size_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t",
          "cbuffer", "const", "export", "groupshared", "in", "inline", "inout", "out", "packoffset",
          "register", "shared", "static", "struct", "texture", "texture1D", "texture2D", "texture3D",
          "textureCube", "texture1DArray", "texture2DArray", "texture3DArray", "textureCubeArray",
          "RWTexture1D", "RWTexture2D", "RWTexture3D", "RWTexture1DArray", "RWTexture2DArray",
          "Buffer", "ByteAddressBuffer", "RWBuffer", "RWByteAddressBuffer",
          "sampler", "sampler1D", "sampler2D", "sampler3D", "samplerCUBE", "sampler_state",
          "SamplerState", "SamplerComparisonState",
          "SV_GroupIndex", "SV_GroupThreadID", "SV_DispatchThreadID", "SV_DomainLocation",
          "SV_GroupID", "SV_OutputControlPointID", "SV_Position", "SV_PrimitiveID",
          "SV_TessFactor", "SV_InsideTessFactor", "SV_InstanceID", "SV_VertexID",
          "SV_RenderTargetArrayIndex", "SV_ViewportArrayIndex", "SV_ClipDistance", "SV_CullDistance",
          "SV_Coverage", "SV_Depth", "SV_Depth0", "SV_Depth1", "SV_Depth2", "SV_Depth3", "SV_Depth4", "SV_Depth5", "SV_Depth6", "SV_Depth7",
          "SV_StencilRef", "SV_Target0", "SV_Target1", "SV_Target2", "SV_Target3", "SV_Target4", "SV_Target5", "SV_Target6", "SV_Target7",
          "SV_IsFrontFace", "SV_StencilRef",
          "vector", "matrix", "float1", "float2", "float3", "float4", "float2x2", "float3x3", "float4x4",
          "double1", "double2", "double3", "double4", "double2x2", "double3x3", "double4x4",
          "int1", "int2", "int3", "int4", "int2x2", "int3x3", "int4x4",
          "uint1", "uint2", "uint3", "uint4", "uint2x2", "uint3x3", "uint4x4",
          "bool1", "bool2", "bool3", "bool4", "bool2x2", "bool3x3", "bool4x4",
          "float1x1", "float2x1", "float3x1", "float4x1",
          "double1x1", "double2x1", "double3x1", "double4x1",
          "int1x1", "int2x1", "int3x1", "int4x1",
          "uint1x1", "uint2x1", "uint3x1", "uint4x1",
          "asm", "asm_fragment", "compile", "compile_fragment", "const_cast", "decltype", "delete",
          "dynamic_cast", "explicit", "friend", "goto", "inline", "mutable", "namespace", "new",
          "noinline", "operator", "private", "protected", "public", "reinterpret_cast", "sizeof",
          "template", "this", "typedef", "typeid", "typename", "using", "virtual", "volatile",
          "abs", "all", "any", "asdouble", "asfloat", "asin", "asint", "asuint", "atan", "atan2",
          "ceil", "clamp", "clip", "cos", "cosh", "countbits", "cross", "degrees", "determinant",
          "distance", "dot", "exp", "exp2", "faceforward", "findfirstbit", "findlsb", "findmsb",
          "floor", "fmod", "frac", "frexp", "isfinite", "isinf", "isnan", "ldexp", "length",
          "lerp", "log", "log10", "log2", "logg", "mad", "max", "min", "modf", "mul", "normalize",
          "pow", "radians", "rcp", "reflect", "refract", "reverseBits", "round", "rsqrt", "saturate",
          "sign", "sin", "sincos", "sinh", "sqrt", "step", "tan", "tanh", "tex1D", "tex1Dgrad",
          "tex1Dlod", "tex1Dproj", "tex2D", "tex2Dgrad", "tex2Dlod", "tex2Dproj", "tex3D",
          "tex3Dgrad", "tex3Dlod", "tex3Dproj", "texCUBE", "texCUBEgrad", "texCUBElod", "texCUBEproj",
          "transpose", "trunc" });

    // Register restricted tokens in HLSL
    StringMap tokens;
    tokens["__"] = "_";
    tokens["sv_"] = "sv";  // System value prefixes
    tokens["_"] = "_";    // Prevent issues with underscore prefix
    registerInvalidTokens(tokens);

    //
    // Register syntax handlers for each data type.
    //

    registerTypeSyntax(
        Type::FLOAT,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "float",
            "0.0",
            "0.0"));

    registerTypeSyntax(
        Type::FLOATARRAY,
        std::make_shared<HlslFloatArrayTypeSyntax>(
            this,
            "float"));

    registerTypeSyntax(
        Type::INTEGER,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "int",
            "0",
            "0"));

    registerTypeSyntax(
        Type::INTEGERARRAY,
        std::make_shared<HlslIntegerArrayTypeSyntax>(
            this,
            "int"));

    registerTypeSyntax(
        Type::BOOLEAN,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "bool",
            "false",
            "false"));

    registerTypeSyntax(
        Type::COLOR3,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "float3",
            "float3(0.0, 0.0, 0.0)",
            "float3(0.0, 0.0, 0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::COLOR4,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "float4",
            "float4(0.0, 0.0, 0.0, 0.0)",
            "float4(0.0, 0.0, 0.0, 0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR2,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "float2",
            "float2(0.0, 0.0)",
            "float2(0.0, 0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC2_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR3,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "float3",
            "float3(0.0, 0.0, 0.0)",
            "float3(0.0, 0.0, 0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR4,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "float4",
            "float4(0.0, 0.0, 0.0, 0.0)",
            "float4(0.0, 0.0, 0.0, 0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::MATRIX33,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "float3x3",
            "float3x3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)",
            "float3x3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)"));

    registerTypeSyntax(
        Type::MATRIX44,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "float4x4",
            "float4x4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0)",
            "float4x4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0)"));

    registerTypeSyntax(
        Type::STRING,
        std::make_shared<HlslStringTypeSyntax>(this));

    registerTypeSyntax(
        Type::FILENAME,
        std::make_shared<ScalarTypeSyntax>(
            this,
            "Texture2D",
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::BSDF,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "BSDF",
            "BSDF(float3(0.0, 0.0, 0.0), float3(1.0, 1.0, 1.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct BSDF { float3 response; float3 throughput; };"));

    registerTypeSyntax(
        Type::EDF,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "EDF",
            "EDF(float3(0.0, 0.0, 0.0))",
            "EDF(float3(0.0, 0.0, 0.0))",
            "float3",
            "#define EDF float3"));

    registerTypeSyntax(
        Type::VDF,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "BSDF",
            "BSDF(float3(0.0, 0.0, 0.0), float3(1.0, 1.0, 1.0))",
            EMPTY_STRING));

    registerTypeSyntax(
        Type::SURFACESHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "surfaceshader",
            "surfaceshader(float3(0.0, 0.0, 0.0), float3(0.0, 0.0, 0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct surfaceshader { float3 color; float3 transparency; };"));

    registerTypeSyntax(
        Type::VOLUMESHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "volumeshader",
            "volumeshader(float3(0.0, 0.0, 0.0), float3(0.0, 0.0, 0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct volumeshader { float3 color; float3 transparency; };"));

    registerTypeSyntax(
        Type::DISPLACEMENTSHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "displacementshader",
            "displacementshader(float3(0.0, 0.0, 0.0), 1.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct displacementshader { float3 offset; float scale; };"));

    registerTypeSyntax(
        Type::LIGHTSHADER,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "lightshader",
            "lightshader(float3(0.0, 0.0, 0.0), float3(0.0, 0.0, 0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct lightshader { float3 intensity; float3 direction; };"));

    registerTypeSyntax(
        Type::MATERIAL,
        std::make_shared<AggregateTypeSyntax>(
            this,
            "material",
            "material(float3(0.0, 0.0, 0.0), float3(0.0, 0.0, 0.0))",
            EMPTY_STRING,
            "surfaceshader",
            "#define material surfaceshader"));
}

bool HlslSyntax::typeSupported(const TypeDesc* type) const
{
    return *type != Type::STRING;
}

bool HlslSyntax::remapEnumeration(const string& value, TypeDesc type, const string& enumNames, std::pair<TypeDesc, ValuePtr>& result) const
{
    // Early out if not an enum input.
    if (enumNames.empty())
    {
        return false;
    }

    // Don't convert already supported types
    if (type != Type::STRING)
    {
        return false;
    }

    // Early out if no valid value provided
    if (value.empty())
    {
        return false;
    }

    // For HLSL we always convert to integer,
    // with the integer value being an index into the enumeration.
    result.first = Type::INTEGER;
    result.second = nullptr;

    StringVec valueElemEnumsVec = splitString(enumNames, ",");
    for (size_t i = 0; i < valueElemEnumsVec.size(); i++)
    {
        valueElemEnumsVec[i] = trimSpaces(valueElemEnumsVec[i]);
    }
    auto pos = std::find(valueElemEnumsVec.begin(), valueElemEnumsVec.end(), value);
    if (pos == valueElemEnumsVec.end())
    {
        throw ExceptionShaderGenError("Given value '" + value + "' is not a valid enum value for input.");
    }
    const int index = static_cast<int>(std::distance(valueElemEnumsVec.begin(), pos));
    result.second = Value::createValue<int>(index);

    return true;
}

StructTypeSyntaxPtr HlslSyntax::createStructSyntax(const string& structTypeName, const string& defaultValue,
                                                   const string& uniformDefaultValue, const string& typeAlias,
                                                   const string& typeDefinition) const
{
    return std::make_shared<HlslStructTypeSyntax>(
        this,
        structTypeName,
        defaultValue,
        uniformDefaultValue,
        typeAlias,
        typeDefinition);
}

string HlslStructTypeSyntax::getValue(const Value& value, bool /* uniform */) const
{
    const AggregateValue& aggValue = static_cast<const AggregateValue&>(value);

    string result = aggValue.getTypeString() + "(";

    string separator = "";
    for (const auto& memberValue : aggValue.getMembers())
    {
        result += separator;
        separator = ",";

        const string& memberTypeName = memberValue->getTypeString();
        const TypeDesc memberTypeDesc = _parent->getType(memberTypeName);

        // Recursively use the syntax to generate the output, so we can support nested structs.
        result += _parent->getValue(memberTypeDesc, *memberValue, true);
    }

    result += ")";

    return result;
}

MATERIALX_NAMESPACE_END

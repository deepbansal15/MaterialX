# HLSL Shader Generator Module

## Overview

The MaterialXGenHlsl module provides HLSL (High-Level Shading Language) shader generation for Shader Model 5.0, enabling MaterialX materials to be compiled to DirectX 11/12 compatible shaders.

## Features

- **Shader Model 5.0**: Generates HLSL shaders compatible with DirectX 11/12
- **Complete Type Mapping**: Maps MaterialX types to appropriate HLSL types (float3, float4, etc.)
- **Resource Binding**: Uses cbuffer, Texture2D, and SamplerState with register() bindings
- **Semantic Support**: Proper HLSL semantics (SV_Position, SV_Target0, etc.)
- **PBR Support**: Includes environment lighting, shadow mapping, and transmission
- **Standard Library**: Complete HLSL implementation of math and utility functions

## Generated Shader Structure

### Vertex Shader (VS_Main)
- Input: VSInput struct with POSITION, NORMAL, TEXCOORD0 semantics
- Output: VSOutput struct with SV_Position semantic
- Functions: World position transformation, view-projection transformation

### Pixel Shader (PS_Main)  
- Input: VSOutput struct
- Output: PSOutput struct with SV_Target0 semantic
- Functions: Material shading, lighting, transparency handling

## Usage

```cpp
#include <MaterialXGenHlsl/HlslShaderGenerator.h>

// Create HLSL generator
auto hlslGenerator = HlslShaderGenerator::create();

// Create generation context
GenContext context(hlslGenerator);

// Generate shader from MaterialX element
ShaderPtr shader = hlslGenerator->generate("MyShader", materialElement, context);

// Get generated code
const string& vsCode = shader->getStage(Stage::VERTEX)->getSourceCode();
const string& psCode = shader->getStage(Stage::PIXEL)->getSourceCode();
```

## Type Mapping

| MaterialX Type | HLSL Type | Notes |
|----------------|-----------|-------|
| float | float | Scalar |
| float2 | float2 | 2D vector |
| float3 | float3 | 3D vector |
| float4 | float4 | 4D vector |
| color3 | float3 | RGB color |
| color4 | float4 | RGBA color |
| matrix33 | float3x3 | 3×3 matrix |
| matrix44 | float4x4 | 4×4 matrix |
| filename | Texture2D | Texture resource |

## Resource Binding

### Constant Buffers
```hlsl
cbuffer MaterialParams : register(b0)
{
    float4 baseColor;
    float roughness;
    float metallic;
};
```

### Textures and Samplers
```hlsl
Texture2D<float4> albedoTexture : register(t0);
SamplerState linearSampler : register(s0);
```

## Shader Semantics

- **Vertex Input**: POSITION, NORMAL, TANGENT, TEXCOORD0
- **Vertex Output**: SV_Position, TEXCOORD0, etc.
- **Pixel Output**: SV_Target0 (color), SV_Depth (depth)

## Limitations

- HLSL strings not supported (use integer enums instead)
- Some GLSL-only features may require platform-specific handling
- Shader Model 5.0 required (no legacy HLSL support)

## Dependencies

- MaterialXGenHw
- MaterialXGenShader
- MaterialXCore

## Building

The module is automatically built when MATERIALX_BUILD_GENHLSL is enabled in CMake.

## Integration

To use in your project:
1. Link against MaterialXGenHlsl library
2. Include MaterialXGenHlsl/HlslShaderGenerator.h
3. Create HlslShaderGenerator instance
4. Use GenContext for shader generation

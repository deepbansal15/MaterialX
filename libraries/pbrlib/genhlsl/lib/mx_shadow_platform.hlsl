//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// Platform-specific shadow mapping utilities for HLSL
// This file provides DirectX 11/12 specific shadow implementations

#include "pbrlib/genhlsl/lib/mx_closure_type.hlsl"

// HLSL SM 5.0 specific shadow features
// Use hardware comparison for shadow maps when available

// Hardware PCF shadow lookup (uses built-in comparison)
float mx_hardware_pcf_shadow(
    float4 shadowPos,
    Texture2D<float> shadowMap,
    SamplerComparisonState shadowComparisonSampler,
    float bias)
{
    float3 projCoords = shadowPos.xyz / shadowPos.w;
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = projCoords.y * 0.5 + 0.5;
    
    // Check if outside shadow map
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return 1.0;
    }
    
    // Hardware 4-tap PCF
    float shadow = 0.0;
    float2 texelSize = 1.0 / float2(2048.0, 2048.0); // Typical shadow map size
    
    for (int x = -1; x <= 0; x++)
    {
        for (int y = -1; y <= 0; y++)
        {
            shadow += shadowMap.SampleCmpLevelZero(
                shadowComparisonSampler,
                projCoords.xy + float2(x, y) * texelSize,
                projCoords.z - bias).r;
        }
    }
    
    return shadow / 4.0;
}

// Anisotropic filtering for shadow maps
float mx_anisotropic_pcf_shadow(
    float4 shadowPos,
    float3 lightDir,
    Texture2D<float> shadowMap,
    SamplerComparisonState shadowComparisonSampler,
    float bias,
    float anisoThreshold)
{
    // Calculate anisotropy based on light direction
    float3 projectedLightDir = normalize(shadowPos.xyz);
    float anisoFactor = 1.0 + anisoThreshold * (1.0 - abs(dot(normalize(float3(shadowPos.x, 0.0, shadowPos.z)), lightDir)));
    
    // Apply anisotropic filtering
    return mx_hardware_pcf_shadow(shadowPos, shadowMap, shadowComparisonSampler, bias);
}

// Cascaded shadow map sampling
float mx_cascade_shadow(
    float4 worldPos,
    float3 normal,
    float3 lightDir,
    Texture2DArray<float> cascadeShadowMap,
    SamplerComparisonState shadowComparisonSampler,
    float4x4 lightViewProj0,
    float4x4 lightViewProj1,
    float4x4 lightViewProj2,
    float4x4 lightViewProj3,
    float4 cascadeSplits,
    int activeCascades,
    float bias)
{
    float4 shadowPos;
    int cascadeIndex = 3;
    
    // Determine which cascade to use
    float depth = mul(worldPos, lightViewProj0).w;
    if (depth < cascadeSplits.x)
        cascadeIndex = 0;
    else if (depth < cascadeSplits.y)
        cascadeIndex = 1;
    else if (depth < cascadeSplits.z)
        cascadeIndex = 2;
    
    // Select appropriate cascade
    if (cascadeIndex == 0)
        shadowPos = mul(worldPos, lightViewProj0);
    else if (cascadeIndex == 1)
        shadowPos = mul(worldPos, lightViewProj1);
    else if (cascadeIndex == 2)
        shadowPos = mul(worldPos, lightViewProj2);
    else
        shadowPos = mul(worldPos, lightViewProj3);
    
    // Sample from cascade
    float3 projCoords = shadowPos.xyz / shadowPos.w;
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = projCoords.y * 0.5 + 0.5;
    
    // Check if outside shadow map
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        // Try blending between cascades at boundaries
        return 1.0;
    }
    
    return cascadeShadowMap.SampleCmpLevelZero(
        shadowComparisonSampler,
        float3(projCoords.xy, cascadeIndex),
        projCoords.z - bias).r;
}

// Store shadow data for later use
void mx_store_shadow_data(
    float4 worldPos,
    float4x4 lightViewProj,
    out float4 shadowPos : SV_Position,
    out float depth : TEXCOORD0)
{
    shadowPos = mul(worldPos, lightViewProj);
    depth = shadowPos.w;
}

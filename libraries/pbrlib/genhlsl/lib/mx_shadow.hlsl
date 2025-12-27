//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "pbrlib/genhlsl/lib/mx_closure_type.hlsl"
#include "stdlib/genhlsl/lib/mx_math.hlsl"

// Shadow mapping utilities for HLSL
// These functions provide shadow computation for various shadow types

// PCF (Percentage Closer Filtering) for soft shadows
float mx_pcf_shadow(
    float4 shadowPos,
    Texture2D<float> shadowMap,
    SamplerState shadowSampler,
    float bias,
    float2 offset)
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
    
    // PCF sampling
    float shadow = 0.0;
    float2 texelSize = 1.0 / float2(1024.0, 1024.0); // Default shadow map size
    
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float pcfDepth = shadowMap.Sample(shadowSampler, projCoords.xy + float2(x, y) * texelSize + offset).r;
            shadow += projCoords.z - bias <= pcfDepth ? 1.0 : 0.0;
        }
    }
    
    return shadow / 9.0;
}

// Variance shadow map comparison
float mx_vsm_shadow(
    float4 shadowPos,
    Texture2D<float2> vsmMap,
    SamplerState vsmSampler,
    float minVariance)
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
    
    float2 moments = vsmMap.Sample(vsmSampler, projCoords.xy).rg;
    
    float depth = projCoords.z;
    float E_x2 = moments.y;
    float Ex = moments.x;
    float variance = E_x2 - Ex * Ex;
    variance = max(variance, minVariance);
    
    float d = depth - Ex;
    float p_max = variance / (variance + d * d);
    
    return p_max;
}

// Exponential shadow map comparison
float mx_esm_shadow(
    float4 shadowPos,
    Texture2D<float> esmMap,
    SamplerState esmSampler,
    float compareValue,
    float offset)
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
    
    float shadowMapDepth = esmMap.Sample(esmSampler, projCoords.xy).r;
    
    // ESM formula: exp(c * (depth - map_depth))
    float shadow = saturate(exp(offset * (compareValue - shadowMapDepth)));
    
    return shadow;
}

// Compute shadow factor for directional light
float mx_compute_directional_shadow(
    float4 worldPos,
    float3 normal,
    float3 lightDir,
    Texture2D<float> shadowMap,
    SamplerState shadowSampler,
    float4x4 lightViewProj,
    float bias,
    float2 pcfOffset)
{
    float4 shadowPos = mul(float4(worldPos.xyz, 1.0), lightViewProj);
    return mx_pcf_shadow(shadowPos, shadowMap, shadowSampler, bias, pcfOffset);
}

// Compute shadow factor for point light (cube map)
float mx_compute_point_shadow(
    float3 worldPos,
    float3 lightPos,
    float3 normal,
    TextureCube<float> shadowCube,
    SamplerState shadowSampler,
    float bias)
{
    float3 lightToFrag = worldPos - lightPos;
    float depth = length(lightToFrag);
    
    // Sample shadow cube map
    float shadowDepth = shadowCube.Sample(shadowSampler, lightToFrag).r;
    
    return depth - bias <= shadowDepth ? 1.0 : 0.0;
}

// Compute shadow factor for spot light
float mx_compute_spot_shadow(
    float4 worldPos,
    float3 normal,
    float3 lightDir,
    Texture2D<float> shadowMap,
    SamplerState shadowSampler,
    float4x4 lightViewProj,
    float bias,
    float2 pcfOffset)
{
    return mx_compute_directional_shadow(worldPos, normal, lightDir,
        shadowMap, shadowSampler, lightViewProj, bias, pcfOffset);
}

// Depth moments computation for VSM
float2 mx_compute_depth_moments(float depth)
{
    float moment1 = depth;
    float moment2 = depth * depth;
    
    return float2(moment1, moment2);
}

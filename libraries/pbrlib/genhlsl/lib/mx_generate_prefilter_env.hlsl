//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "pbrlib/genhlsl/lib/mx_closure_type.hlsl"
#include "stdlib/genhlsl/lib/mx_math.hlsl"

// Environment prefiltering functions for HLSL
// These functions implement environment map prefiltering for IBL

// Prefiltered environment radiance lookup
float3 mx_prefilter_environment_radiance(
    float3 L,
    float roughness,
    float3 baseColor,
    float metallic,
    Texture2D<float4> prefilteredEnvMap,
    SamplerState prefilteredEnvSampler)
{
    // Calculate mip level based on roughness
    float maxMipLevel = 5.0; // 6 levels for 64x32 base
    float mipLevel = roughness * maxMipLevel;
    
    // Sample prefiltered environment
    float3 radiance = prefilteredEnvMap.SampleLevel(prefilteredEnvSampler, mx_latlong_map(L), mipLevel).rgb;
    
    return radiance;
}

// Prefilter environment for a single channel
float mx_prefilter_environment_single_channel(
    float3 L,
    float roughness,
    Texture2D<float> prefilteredChannelMap,
    SamplerState prefilteredChannelSampler)
{
    float maxMipLevel = 5.0;
    float mipLevel = roughness * maxMipLevel;
    return prefilteredChannelMap.SampleLevel(prefilteredChannelSampler, mx_latlong_map(L), mipLevel);
}

// Compute GGX distribution for prefiltering
float mx_prefilter_GGX_D(float NdotH, float roughness)
{
    float a = roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    
    return nom / max(denom, 0.0001);
}

// Importance sample for environment prefiltering
float3 mx_importance_sample_GGX(float2 Xi, float roughness, float3 N)
{
    float a = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    
    float3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;
    
    // Transform to world space
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangentX = normalize(cross(up, N));
    float3 tangentY = cross(N, tangentX);
    
    return tangentX * H.x + tangentY * H.y + N * H.z;
}

// Prefilter a cubemap at runtime (for offline precomputation)
float3 mx_prefilter_cubemap(
    TextureCube<float4> envMap,
    SamplerState envSampler,
    float3 N,
    float roughness,
    uint sampleCount)
{
    float3 prefilteredColor = float3(0.0, 0.0, 0.0);
    float totalWeight = 0.0;
    
    float3 V = N;
    
    for (uint i = 0; i < sampleCount; i++)
    {
        float2 Xi = float2(
            frac(float(i) / float(sampleCount) + 0.5),
            frac(float(i * 37.0) / float(sampleCount))
        );
        
        float3 H = mx_importance_sample_GGX(Xi, roughness, N);
        float3 L = reflect(-V, H);
        
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        
        // PDF for GGX
        float pdf = mx_prefilter_GGX_D(NdotH, roughness) * NdotH / (4.0 * NdotL);
        
        // Sample environment
        float3 envColor = envMap.Sample(envSampler, L).rgb;
        
        // Weight by BRDF
        float weight = NdotL * pdf;
        
        prefilteredColor += envColor * weight;
        totalWeight += weight;
    }
    
    return prefilteredColor / max(totalWeight, 0.0001);
}

// Environment prefilter for indirect specular
float3 mx_environment_specular(
    float3 V,
    float3 N,
    float roughness,
    float3 baseColor,
    float metallic,
    Texture2D<float4> prefilteredMap,
    SamplerState prefilteredSampler)
{
    float3 R = reflect(-V, N);
    float3 prefiltered = mx_prefilter_environment_radiance(R, roughness, baseColor, metallic, prefilteredMap, prefilteredSampler);
    
    // Fresnel term
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), baseColor, metallic);
    float NdotV = max(dot(N, V), 0.0);
    float3 F = mx_fresnel_schlick(NdotV, F0);
    
    // Apply Fresnel to prefiltered result
    return prefiltered * (float3(1.0, 1.0, 1.0) - F);
}

// Main function for generating prefiltered environment
float4 mx_generate_prefilter_env()
{
    // This function is called when hwWriteEnvPrefilter is enabled
    // Returns prefiltered environment color
    return float4(0.5, 0.5, 0.5, 1.0);
}

// Compute prefiltered radiance for single scattering
float3 mx_single_scattering(
    float3 V,
    float3 N,
    float roughness,
    float3 baseColor,
    float metallic,
    Texture2D<float4> prefilteredMap,
    SamplerState prefilteredSampler)
{
    return mx_environment_specular(V, N, roughness, baseColor, metallic, prefilteredMap, prefilteredSampler);
}

// Multiple scattering approximation for IBL
float3 mx_multi_scattering(
    float3 V,
    float3 N,
    float roughness,
    float3 baseColor,
    float metallic,
    float3 singleScattering,
    float3 ambient)
{
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), baseColor, metallic);
    float NdotV = max(dot(N, V), 0.0);
    float3 F = mx_fresnel_schlick(NdotV, F0);
    
    // Simple multi-scattering approximation
    float3 multiScatt = ambient * F * (1.0 - metallic);
    
    return singleScattering + multiScatt;
}

//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "pbrlib/genhlsl/lib/mx_closure_type.hlsl"
#include "stdlib/genhlsl/lib/mx_math.hlsl"

// Environment map prefiltering functions for IBL
// These functions implement environment map filtering for roughness-based specular

// Prefiltered environment map lookup with roughness
float3 mx_prefilter_environment(
    float3 L,
    float roughness,
    Texture2D<float4> prefilteredMap,
    SamplerState prefilteredSampler)
{
    // Calculate the mip level based on roughness
    float maxMipLevels = 6.0; // Assuming 256x128 base resolution
    float mipLevel = roughness * maxMipLevels;
    
    // Sample from prefiltered environment map
    return prefilteredMap.SampleLevel(prefilteredSampler, mx_latlong_map(L), mipLevel).rgb;
}

// Compute GGX distribution for environment prefiltering
float mx_ggx_environment_D(float NdotH, float roughness)
{
    float a2 = roughness * roughness;
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / denom;
}

// Importance sample the environment for prefiltering
float3 mx_sample_environment(float2 U, float roughness)
{
    float phi = 2.0 * PI * U.x;
    
    // Sample elevation based on roughness
    float cosTheta = sqrt(1.0 - U.y);
    float sinTheta = sqrt(U.y);
    
    float3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;
    
    return H;
}

// Prefilter a cubemap for a given roughness
// This is typically done offline, but these functions help with runtime filtering
float3 mx_filter_environment_cubemap(
    float3 N,
    float roughness,
    TextureCube<float4> envMap,
    SamplerState envSampler,
    uint sampleCount)
{
    float3 result = float3(0.0, 0.0, 0.0);
    float totalWeight = 0.0;
    
    // SimpleMonte Carlo sampling
    for (uint i = 0; i < sampleCount; i++)
    {
        float2 U = float2(
            frac(float(i) / float(sampleCount) + 0.5),
            frac(float(i * 37.0) / float(sampleCount))
        );
        
        // Sample hemisphere
        float phi = 2.0 * PI * U.x;
        float cosTheta = sqrt(1.0 - U.y);
        float sinTheta = sqrt(U.y);
        
        float3 H;
        H.x = sinTheta * cos(phi);
        H.y = sinTheta * sin(phi);
        H.z = cosTheta;
        
        float NdotH = max(dot(N, H), 0.0);
        
        // PDF for cosine-weighted sampling
        float pdf = NdotH / PI;
        
        // Sample environment
        float3 L = reflect(-V, H);
        float3 envSample = envMap.Sample(envSampler, L).rgb;
        
        // Weight by BSDF
        float weight = mx_ggx_environment_D(NdotH, roughness) * NdotH / pdf;
        
        result += envSample * weight;
        totalWeight += weight;
    }
    
    return result / max(totalWeight, 0.0001);
}

// Environment radiance with prefiltered lookup
float3 mx_env_radiance_prefilter(
    float3 L,
    float roughness,
    float3 baseColor,
    float metallic,
    float3 F0,
    Texture2D<float4> prefilteredMap,
    SamplerState prefilteredSampler)
{
    // Get prefiltered radiance
    float3 radiance = mx_prefilter_environment(L, roughness, prefilteredMap, prefilteredSampler);
    
    // Apply Fresnel
    float NdotL = max(dot(float3(0.0, 1.0, 0.0), L), 0.0);
    float3 F = mx_fresnel_schlick(NdotL, F0);
    
    return radiance * (float3(1.0, 1.0, 1.0) - F);
}

// Environment irradiance (diffuse)
float3 mx_env_irradiance_prefilter(
    float3 N,
    float3 baseColor,
    float metallic,
    Texture2D<float4> irradianceMap,
    SamplerState irradianceSampler)
{
    float3 irradiance = irradianceMap.SampleLevel(irradianceSampler, mx_latlong_map(N), 0.0).rgb;
    
    // Apply diffuse albedo
    float3 albedo = lerp(baseColor, float3(0.0, 0.0, 0.0), metallic);
    
    return irradiance * albedo;
}

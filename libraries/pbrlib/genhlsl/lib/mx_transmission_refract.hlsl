//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "pbrlib/genhlsl/lib/mx_closure_type.hlsl"
#include "stdlib/genhlsl/lib/mx_math.hlsl"

// Refraction-based transmission rendering for HLSL
// Implements physical-based refraction using Snell's law

// Calculate IOR (Index of Refraction) from eta
float mx_ior_to_eta(float ior)
{
    return 1.0 / ior;
}

// Calculate Fresnel reflectance for dielectrics using Schlick's approximation
float mx_fresnel_dielectric(float NdotV, float ior)
{
    float f0 = abs((1.0 - ior) / (1.0 + ior));
    f0 = f0 * f0;
    return f0 + (1.0 - f0) * pow(1.0 - NdotV, 5.0);
}

// Fresnel reflectance for conductors
float3 mx_fresnel_conductor(float3 eta, float3 k, float cosTheta)
{
    float3 F0 = (eta * eta + k * k) / (eta * eta + k * k + float3(1.0, 1.0, 1.0));
    float3 oneMinusF0 = float3(1.0, 1.0, 1.0) - F0;
    return F0 + oneMinusF0 * pow(1.0 - cosTheta, 5.0);
}

// Refraction direction using Snell's law
float3 mx_refract(float3 I, float3 N, float eta)
{
    float NdotI = dot(N, I);
    float k = 1.0 - eta * eta * (1.0 - NdotI * NdotI);
    if (k < 0.0)
        return float3(0.0, 0.0, 0.0); // Total internal reflection
    return eta * I - (eta * NdotI + sqrt(k)) * N;
}

// Compute transmission color with absorption (Beer's law)
float3 mx_transmission_color(float3 baseColor, float thickness, float3 absorption)
{
    return exp(-thickness * absorption * (float3(1.0, 1.0, 1.0) - baseColor));
}

// Evaluate transmission BSDF for refraction
float3 mx_transmission_bsdf(
    float3 N,
    float3 V,
    float3 L,
    float3 H,
    float NdotL,
    float NdotV,
    float NdotH,
    float3 baseColor,
    float roughness,
    float ior,
    float3 transmissionColor,
    float thickness,
    float3 absorption)
{
    // Fresnel term
    float F = mx_fresnel_dielectric(NdotV, ior);
    
    // Microfacet distribution
    float D = mx_ggx_D(NdotH, roughness);
    
    // Geometric shadowing
    float G = mx_ggx_smith_G2(NdotL, NdotV, roughness);
    
    // Transmission factor
    float3 transmission = mx_transmission_color(baseColor, thickness, absorption);
    
    // Combine factors
    float3 bsdf = (float3(1.0, 1.0, 1.0) - F) * transmission * D * G / (4.0 * NdotL * NdotV);
    
    return bsdf;
}

// Simple refraction approximation for thin surfaces
float3 mx_thin_refraction(
    float3 N,
    float3 V,
    float3 baseColor,
    float ior,
    Texture2D<float4> transmissionMap,
    SamplerState transmissionSampler,
    float2 uv)
{
    // Calculate refraction vector
    float3 refracted = refract(-V, N, mx_ior_to_eta(ior));
    
    // Sample transmission map with refracted vector
    // For simplicity, using same UV coordinates (would need cubemap for full refraction)
    float3 transmission = transmissionMap.Sample(transmissionSampler, uv).rgb;
    
    // Apply base color tint
    return transmission * baseColor;
}

// Volume absorption for thick transparent materials
float3 mx_volume_absorption(
    float3 baseColor,
    float distance,
    float3 absorption)
{
    return baseColor * exp(-distance * absorption);
}

// Environment refraction (refract environment map lookup)
float3 mx_refract_environment(
    float3 N,
    float3 V,
    float ior,
    Texture2D<float4> envMap,
    SamplerState envSampler)
{
    float3 R = refract(-V, N, mx_ior_to_eta(ior));
    if (length(R) < 0.001)
    {
        // Total internal reflection - reflect instead
        R = reflect(-V, N);
    }
    return envMap.Sample(envSampler, mx_latlong_map(R)).rgb;
}

// Combined reflection/refraction for dielectric materials
float4 mx_reflection_refraction(
    float3 N,
    float3 V,
    float3 baseColor,
    float roughness,
    float ior,
    float3 reflectionColor,
    float3 transmissionColor,
    Texture2D<float4> envMap,
    SamplerState envSampler)
{
    float NdotV = max(dot(N, V), 0.0);
    
    // Fresnel
    float F = mx_fresnel_dielectric(NdotV, ior);
    
    // Reflection
    float3 R = reflect(-V, N);
    float3 reflection = envMap.Sample(envSampler, mx_latlong_map(R)).rgb;
    
    // Refraction
    float3 refracted = refract(-V, N, mx_ior_to_eta(ior));
    float3 refraction;
    if (length(refracted) < 0.001)
    {
        // Total internal reflection
        refraction = reflection;
    }
    else
    {
        refraction = envMap.Sample(envSampler, mx_latlong_map(refracted)).rgb;
    }
    
    // Combine with base color
    refraction *= baseColor * transmissionColor;
    
    return float4(lerp(refraction, reflection, F), 1.0);
}

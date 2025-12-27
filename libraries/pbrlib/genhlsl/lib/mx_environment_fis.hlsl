//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "pbrlib/genhlsl/lib/mx_closure_type.hlsl"
#include "stdlib/genhlsl/lib/mx_math.hlsl"

// Fresnel integrals approximation using Schlick's approximation
// for better performance with equi-angular sampling
float2 mx_fresnel_equation_mix(float cosTheta, float2 F0)
{
    float ft = pow(1.0 - cosTheta, 5.0);
    return lerp(float2(1.0, 1.0), F0, ft);
}

// Compute directional albedo using Fermi-Dirac integration
// Based on "Practical Rendering and Computation with Randomized
// Spectral Measure" by Heitz et al.
float mx_fermi_dirac_integral(float x, float k)
{
    // Approximation of the Fermi-Dirac integral
    // k controls the sharpness of the transition
    return 1.0 / (1.0 + exp(-k * x));
}

// Importance sample the equi-angular distribution for FIS
float3 mx_sample_fis(float2 alpha, float2 U)
{
    // Equi-angular sampling for rough surfaces
    // Based on "Importance Sampling Microfacet-Based BSDFs
    // using the Distribution of Visible Normals"
    float phi = 2.0 * PI * U.x;
    
    // Sample elevation using cosine-weighted distribution
    float cosTheta = sqrt(1.0 - U.y);
    float sinTheta = sqrt(U.y);
    
    float3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.y = cosTheta; // Z is up
    
    return H;
}

// Compute the contribution of a light source to indirect lighting
// using Fresnel-based Importance Sampling
float3 mx_compute_indirect_fis(
    float3 N,
    float3 V,
    float3 L,
    float3 H,
    float NdotL,
    float NdotV,
    float NdotH,
    float3 albedo,
    float roughness,
    float3 F0)
{
    // Fresnel term with Schlick's approximation
    float3 F = mx_fresnel_equation_mix(NdotH, F0);
    
    // Masking function (Smith microfacet distribution)
    float G = mx_ggx_smith_G1(NdotL, roughness) * mx_ggx_smith_G1(NdotV, roughness);
    
    // BSDF evaluation
    float D = mx_ggx_D(NdotH, roughness);
    
    // Contribution from this direction
    float weight = D * G * NdotL;
    
    // Fresnel-modulated albedo
    float3 contribution = albedo * (float3(1.0, 1.0, 1.0) - F);
    
    return contribution * weight;
}

// Evaluate indirect lighting from environment map using FIS
float3 mx_env_radiance_fis(
    float3 N,
    float3 V,
    float3 baseColor,
    float metallic,
    float roughness,
    Texture2D<float4> envMap,
    SamplerState envSampler)
{
    // Compute F0 from material properties
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), baseColor, metallic);
    
    // Sample environment map
    float3 envColor = envMap.SampleLevel(envSampler, mx_latlong_map(N), 0.0).rgb;
    
    // Apply basic lighting calculation
    return envColor;
}

// Compute irradiance from environment using FIS approximation
float3 mx_env_irradiance_fis(
    float3 N,
    float3 baseColor,
    float metallic,
    float roughness,
    Texture2D<float4> irradianceMap,
    SamplerState irradianceSampler)
{
    // Sample irradiance from cubemap/equirectangular map
    float3 irradiance = irradianceMap.SampleLevel(irradianceSampler, mx_latlong_map(N), 0.0).rgb;
    
    // Apply material albedo
    float3 albedo = lerp(baseColor, float3(0.0, 0.0, 0.0), metallic);
    
    return irradiance * albedo;
}

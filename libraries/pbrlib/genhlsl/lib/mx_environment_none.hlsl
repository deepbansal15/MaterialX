//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include "pbrlib/genhlsl/lib/mx_closure_type.hlsl"

float3 mx_environment_none(float3 N, float3 V, float3 lightDirection, float lightIntensity, float3 lightColor)
{
    return float3(0.0, 0.0, 0.0);
}

float3 mx_env_irradiance_none(float3 N)
{
    return float3(0.0, 0.0, 0.0);
}

float3 mx_env_radiance_none(float3 L, float roughness)
{
    return float3(0.0, 0.0, 0.0);
}

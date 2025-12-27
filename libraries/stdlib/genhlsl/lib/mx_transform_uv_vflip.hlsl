//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// UV transformation with vertical flip for HLSL
// This file provides UV transformation functions that flip the V coordinate

// Apply vertical flip to UV coordinates
float2 mx_transform_uv_vflip(float2 uv, float2 scale, float2 offset)
{
    // Flip the V coordinate (multiply by -1 and add 1)
    float2 flippedUV;
    flippedUV.x = uv.x * scale.x + offset.x;
    flippedUV.y = (1.0 - uv.y) * scale.y + offset.y;
    return flippedUV;
}

// Apply transformation with tiling, offset, and vertical flip
float2 mx_transform_uv_vflip_tiled(float2 uv, float2 transform)
{
    float tiling = transform.x;
    float offset = transform.y;
    
    float2 flippedUV;
    flippedUV.x = frac(uv.x * tiling + offset);
    flippedUV.y = frac((1.0 - uv.y) * tiling + offset);
    return flippedUV;
}

// Transform UV with rotation and vertical flip
float2 mx_transform_uv_vflip_rotated(float2 uv, float rotation, float2 center)
{
    float c = cos(rotation);
    float s = sin(rotation);
    
    // Flip V first
    float2 flippedUV = float2(uv.x, 1.0 - uv.y);
    
    // Rotate around center
    float2 rotatedUV;
    rotatedUV.x = c * (flippedUV.x - center.x) - s * (flippedUV.y - center.y) + center.x;
    rotatedUV.y = s * (flippedUV.x - center.x) + c * (flippedUV.y - center.y) + center.y;
    
    return rotatedUV;
}

// Combined transformation: scale, rotate, translate, and vertical flip
float2 mx_transform_uv_vflip_combined(
    float2 uv,
    float2 scale,
    float rotation,
    float2 offset,
    float2 center)
{
    // First apply vertical flip
    float2 flippedUV = float2(uv.x, 1.0 - uv.y);
    
    // Apply scale
    flippedUV = flippedUV * scale;
    
    // Apply rotation around center
    float c = cos(rotation);
    float s = sin(rotation);
    float2 rotatedUV;
    rotatedUV.x = c * (flippedUV.x - center.x) - s * (flippedUV.y - center.y) + center.x;
    rotatedUV.y = s * (flippedUV.x - center.x) + c * (flippedUV.y - center.y) + center.y;
    
    // Apply offset
    rotatedUV += offset;
    
    return rotatedUV;
}

//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// Hexagonal tiling utilities for HLSL
// This file provides functions for hex tile pattern generation

#include "stdlib/genhlsl/lib/mx_math.hlsl"

// Convert UV to hex grid coordinates
float2 mx_hex_grid_uv(float2 uv, float2 size)
{
    float2 scaledUV = uv * size;
    return scaledUV;
}

// Get hex cell ID and local coordinates
float4 mx_hex_cell(float2 uv, float2 size)
{
    float2 scaledUV = uv * size;
    
    // Hex dimensions
    float hexWidth = 1.0;
    float hexHeight = sqrt(3.0) / 2.0;
    float horizSpacing = hexWidth * 0.75;
    float vertSpacing = hexHeight;
    
    // Calculate hex coordinates
    float2 gridUV = scaledUV / float2(horizSpacing, vertSpacing);
    
    // Offset every other row
    float row = floor(gridUV.y);
    float col = floor(gridUV.x);
    
    if (frac(row * 0.5) > 0.0)
    {
        gridUV.x += 0.5;
    }
    
    // Cell within hex
    float2 cellUV = float2(fract(gridUV.x), fract(gridUV.y));
    
    // Convert to hex local coordinates
    float2 localUV;
    localUV.x = cellUV.x - 0.5;
    localUV.y = cellUV.y - 0.5;
    
    return float4(col, row, localUV.x, localUV.y);
}

// Sample hex tile pattern
float3 mx_hex_tile(
    float2 uv,
    float2 size,
    float3 tileColor,
    float3 bgColor,
    float lineWidth,
    float3 lineColor)
{
    float2 scaledUV = uv * size;
    
    // Hex dimensions
    float hexWidth = 1.0;
    float hexHeight = sqrt(3.0) / 2.0;
    float horizSpacing = hexWidth * 0.75;
    float vertSpacing = hexHeight;
    
    // Calculate hex grid position
    float2 gridUV = scaledUV / float2(horizSpacing, vertSpacing);
    
    // Offset for staggered rows
    float row = floor(gridUV.y);
    float col = floor(gridUV.x);
    
    // Hex center in local coordinates
    float2 hexCenter;
    if (frac(row * 0.5) > 0.0)
    {
        hexCenter.x = (col + 0.5) * horizSpacing + 0.5 * horizSpacing;
    }
    else
    {
        hexCenter.x = (col + 0.5) * horizSpacing;
    }
    hexCenter.y = (row + 0.5) * vertSpacing;
    
    // Local position within hex
    float2 localPos = float2(mod(scaledUV.x, horizSpacing), mod(scaledUV.y, vertSpacing));
    
    // Hexagon SDF
    float2 hexUV = localPos - float2(horizSpacing / 2.0, vertSpacing / 2.0);
    float r = 0.5 * hexWidth;
    
    // Calculate distance to hex edge
    float3 hexCoords = float3(hexUV.x, hexUV.y + r, -hexUV.x - hexUV.y + r);
    float3 hexDist = abs(hexCoords);
    float dist = max(hexDist.x, max(hexDist.y, hexDist.z)) - r * sqrt(3.0) / 2.0;
    
    // Tile pattern
    float3 color = bgColor;
    
    // Check if inside hex
    if (dist < -lineWidth / 2.0)
    {
        color = tileColor;
    }
    else if (abs(dist) < lineWidth / 2.0)
    {
        color = lineColor;
    }
    
    return color;
}

// Hex tile with position-based variation
float3 mx_hex_tile_varied(
    float2 uv,
    float2 size,
    float3 baseColor,
    float variation,
    Texture2D<float4> variationMap,
    SamplerState variationSampler)
{
    float2 scaledUV = uv * size;
    
    // Calculate hex cell ID
    float hexWidth = 1.0;
    float hexHeight = sqrt(3.0) / 2.0;
    float horizSpacing = hexWidth * 0.75;
    float vertSpacing = hexHeight;
    
    float2 gridUV = scaledUV / float2(horizSpacing, vertSpacing);
    float row = floor(gridUV.y);
    float col = floor(gridUV.x);
    
    // Sample variation based on cell ID
    float2 varUV = float2(
        frac(col * 0.123 + row * 0.456),
        frac(col * 0.789 + row * 0.321)
    );
    float3 varColor = variationMap.Sample(variationSampler, varUV).rgb;
    
    // Apply variation
    float3 tileColor = baseColor * (1.0 - variation + varColor * variation);
    
    // Calculate hex position
    float2 localPos = float2(mod(scaledUV.x, horizSpacing), mod(scaledUV.y, vertSpacing));
    float2 hexUV = localPos - float2(horizSpacing / 2.0, vertSpacing / 2.0);
    float r = 0.5 * hexWidth;
    
    // Hexagon SDF
    float3 hexCoords = float3(hexUV.x, hexUV.y + r, -hexUV.x - hexUV.y + r);
    float3 hexDist = abs(hexCoords);
    float dist = max(hexDist.x, max(hexDist.y, hexDist.z)) - r * sqrt(3.0) / 2.0;
    
    return tileColor;
}

// Normal map for hex tiles
float3 mx_hex_tile_normal(
    float2 uv,
    float2 size,
    float normalStrength)
{
    float2 scaledUV = uv * size;
    
    float hexWidth = 1.0;
    float hexHeight = sqrt(3.0) / 2.0;
    float horizSpacing = hexWidth * 0.75;
    float vertSpacing = hexHeight;
    
    float2 localPos = float2(mod(scaledUV.x, horizSpacing), mod(scaledUV.y, vertSpacing));
    float2 hexUV = localPos - float2(horizSpacing / 2.0, vertSpacing / 2.0);
    float r = 0.5 * hexWidth;
    
    // Calculate normal based on hex shape
    float3 hexCoords = float3(hexUV.x, hexUV.y + r, -hexUV.x - hexUV.y + r);
    float3 hexDist = abs(hexCoords);
    float dist = max(hexDist.x, max(hexDist.y, hexDist.z)) - r * sqrt(3.0) / 2.0;
    
    // Simple normal calculation
    float3 normal = float3(0.0, 0.0, 1.0);
    
    if (dist > -0.01)
    {
        // Edge - tilt normal
        float2 edgeDir = normalize(float2(hexUV.x, hexUV.y));
        normal = normalize(float3(-edgeDir.x * normalStrength, -edgeDir.y * normalStrength, 1.0));
    }
    
    return normal;
}

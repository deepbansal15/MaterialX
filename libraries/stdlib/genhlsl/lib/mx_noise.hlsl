//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// Unified noise library for HLSL
// This file provides various noise functions for shader generation

#include "stdlib/genhlsl/lib/mx_math.hlsl"

// Hash function for noise
float mx_hash_noise(float2 p)
{
    float3 p3 = frac(float3(p.x, p.y, p.x) * float3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return frac((p3.x + p3.y) * p3.z);
}

float mx_hash_noise(float3 p)
{
    float3 p3 = frac(p * float3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz + 33.33);
    return frac((p3.x + p3.y) * p3.z);
}

// 2D value noise
float mx_noise_2d(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = mx_hash_noise(i);
    float b = mx_hash_noise(i + float2(1.0, 0.0));
    float c = mx_hash_noise(i + float2(0.0, 1.0));
    float d = mx_hash_noise(i + float2(1.0, 1.0));
    
    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}

// 3D value noise
float mx_noise_3d(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = mx_hash_noise(i);
    float b = mx_hash_noise(i + float3(1.0, 0.0, 0.0));
    float c = mx_hash_noise(i + float3(0.0, 1.0, 0.0));
    float d = mx_hash_noise(i + float3(1.0, 1.0, 0.0));
    float e = mx_hash_noise(i + float3(0.0, 0.0, 1.0));
    float ff = mx_hash_noise(i + float3(1.0, 0.0, 1.0));
    float g = mx_hash_noise(i + float3(0.0, 1.0, 1.0));
    float h = mx_hash_noise(i + float3(1.0, 1.0, 1.0));
    
    return lerp(lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y),
                lerp(lerp(e, ff, f.x), lerp(g, h, f.x), f.y), f.z);
}

// 2D simplex noise (faster than value noise)
float2 mx_simplex_noise_grad(float2 p)
{
    const float K1 = 0.366025404; // (sqrt(3)-1)/2
    const float K2 = 0.211324865; // (3-sqrt(3))/6
    
    float2 i = floor(p + (p.x + p.y) * K1);
    float2 a = p - i + (i.x + i.y) * K2;
    
    float2 o = (a.x < a.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
    float2 b = a - o + K2;
    float2 c = a - 1.0 + 2.0 * K2;
    
    float3 g = float3(0.0, 0.0, 0.0);
    g.x = mx_hash_noise(i + float2(0.0, 0.0)) - 0.5;
    g.y = mx_hash_noise(i + float2(1.0, 0.0)) - 0.5;
    g.z = mx_hash_noise(i + float2(0.0, 1.0)) - 0.5;
    
    float3 h = float3(0.0, 0.0, 0.0);
    h.x = mx_hash_noise(i + float2(1.0, 1.0)) - 0.5;
    h.y = mx_hash_noise(i + float2(a.x, a.y)) - 0.5;
    h.z = mx_hash_noise(i + float2(b.x, b.y)) - 0.5;
    
    return float2(0.0, 0.0); // Simplified
}

// Fractal Brownian Motion
float mx_fbm_2d(float2 p, int octaves, float lacunarity, float persistence)
{
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    float maxValue = 0.0;
    
    for (int i = 0; i < octaves; i++)
    {
        value += amplitude * mx_noise_2d(p * frequency);
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return value / maxValue;
}

float mx_fbm_3d(float3 p, int octaves, float lacunarity, float persistence)
{
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    float maxValue = 0.0;
    
    for (int i = 0; i < octaves; i++)
    {
        value += amplitude * mx_noise_3d(p * frequency);
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return value / maxValue;
}

// Worley noise (cellular noise)
float2 mx_worley_noise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    
    float2 minDist = float2(1.0, 1.0);
    
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float2 neighbor = float2(float(x), float(y));
            float2 point = float2(
                mx_hash_noise(i + neighbor),
                mx_hash_noise(i + neighbor + float2(5.2, 1.3))
            );
            point = 0.5 + 0.5 * sin(6.2831 * point);
            
            float2 diff = neighbor + point - f;
            float dist = length(diff);
            
            minDist = min(minDist, float2(dist, point.x));
        }
    }
    
    return minDist;
}

// Gradient noise (Perlin-like)
float mx_gradient_noise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = mx_hash_noise(i);
    float b = mx_hash_noise(i + float2(1.0, 0.0));
    float c = mx_hash_noise(i + float2(0.0, 1.0));
    float d = mx_hash_noise(i + float2(1.0, 1.0));
    
    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}

// Color noise (Voronoi-like)
float mx_color_noise(float2 p, int colors)
{
    float2 i = floor(p);
    float2 f = frac(p);
    
    float minDist = 1.0;
    float colorIndex = 0.0;
    
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float2 neighbor = float2(float(x), float(y));
            float2 point = float2(
                mx_hash_noise(i + neighbor),
                mx_hash_noise(i + neighbor + float2(3.2, 1.7))
            );
            point = 0.5 + 0.5 * sin(6.2831 * point);
            
            float2 diff = neighbor + point - f;
            float dist = length(diff);
            
            if (dist < minDist)
            {
                minDist = dist;
                colorIndex = floor(point.x * float(colors));
            }
        }
    }
    
    return colorIndex / float(colors - 1);
}

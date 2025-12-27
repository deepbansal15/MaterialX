// HLSL Common Math Functions for MaterialX
// This file provides HLSL equivalents of GLSL math functions

// ============================================================================
// Basic Math Functions
// ============================================================================

float mx_degrees(float radians)
{
    return degrees(radians);
}

float mx_radians(float degrees)
{
    return radians(degrees);
}

float mx_sin(float angle)
{
    return sin(angle);
}

float mx_cos(float angle)
{
    return cos(angle);
}

float mx_tan(float angle)
{
    return tan(angle);
}

float mx_asin(float x)
{
    return asin(x);
}

float mx_acos(float x)
{
    return acos(x);
}

float mx_atan(float y, float x)
{
    return atan2(y, x);
}

float mx_atan(float y)
{
    return atan(y);
}

float mx_pow(float base, float exponent)
{
    return pow(base, exponent);
}

float mx_exp(float x)
{
    return exp(x);
}

float mx_log(float x)
{
    return log(x);
}

float mx_exp2(float x)
{
    return exp2(x);
}

float mx_log2(float x)
{
    return log2(x);
}

float mx_sqrt(float x)
{
    return sqrt(x);
}

float mx_inversesqrt(float x)
{
    return rsqrt(x);
}

float mx_abs(float x)
{
    return abs(x);
}

float mx_sign(float x)
{
    return sign(x);
}

float mx_floor(float x)
{
    return floor(x);
}

float mx_ceil(float x)
{
    return ceil(x);
}

float mx_frac(float x)
{
    return frac(x);
}

float mx_mod(float x, float y)
{
    return fmod(x, y);
}

float mx_min(float a, float b)
{
    return min(a, b);
}

float mx_max(float a, float b)
{
    return max(a, b);
}

float mx_clamp(float x, float minVal, float maxVal)
{
    return saturate(clamp(x, minVal, maxVal));
}

float mx_mix(float a, float b, float t)
{
    return lerp(a, b, t);
}

float mx_step(float edge, float x)
{
    return step(edge, x);
}

float mx_smoothstep(float edge0, float edge1, float x)
{
    return smoothstep(edge0, edge1, x);
}

// ============================================================================
// Vector Math Functions
// ============================================================================

float2 mx_sin(float2 angle)
{
    return sin(angle);
}

float2 mx_cos(float2 angle)
{
    return cos(angle);
}

float2 mx_tan(float2 angle)
{
    return tan(angle);
}

float2 mx_abs(float2 x)
{
    return abs(x);
}

float2 mx_sign(float2 x)
{
    return sign(x);
}

float2 mx_floor(float2 x)
{
    return floor(x);
}

float2 mx_ceil(float2 x)
{
    return ceil(x);
}

float2 mx_frac(float2 x)
{
    return frac(x);
}

float2 mx_mod(float2 x, float2 y)
{
    return fmod(x, y);
}

float2 mx_min(float2 a, float2 b)
{
    return min(a, b);
}

float2 mx_max(float2 a, float2 b)
{
    return max(a, b);
}

float2 mx_clamp(float2 x, float2 minVal, float2 maxVal)
{
    return saturate(clamp(x, minVal, maxVal));
}

float2 mx_mix(float2 a, float2 b, float2 t)
{
    return lerp(a, b, t);
}

float2 mx_mix(float2 a, float2 b, float t)
{
    return lerp(a, b, t);
}

float2 mx_step(float2 edge, float2 x)
{
    return step(edge, x);
}

float2 mx_smoothstep(float2 edge0, float2 edge1, float2 x)
{
    return smoothstep(edge0, edge1, x);
}

float3 mx_sin(float3 angle)
{
    return sin(angle);
}

float3 mx_cos(float3 angle)
{
    return cos(angle);
}

float3 mx_tan(float3 angle)
{
    return tan(angle);
}

float3 mx_abs(float3 x)
{
    return abs(x);
}

float3 mx_sign(float3 x)
{
    return sign(x);
}

float3 mx_floor(float3 x)
{
    return floor(x);
}

float3 mx_ceil(float3 x)
{
    return ceil(x);
}

float3 mx_frac(float3 x)
{
    return frac(x);
}

float3 mx_mod(float3 x, float3 y)
{
    return fmod(x, y);
}

float3 mx_min(float3 a, float3 b)
{
    return min(a, b);
}

float3 mx_max(float3 a, float3 b)
{
    return max(a, b);
}

float3 mx_clamp(float3 x, float3 minVal, float3 maxVal)
{
    return saturate(clamp(x, minVal, maxVal));
}

float3 mx_mix(float3 a, float3 b, float3 t)
{
    return lerp(a, b, t);
}

float3 mx_mix(float3 a, float3 b, float t)
{
    return lerp(a, b, t);
}

float3 mx_step(float3 edge, float3 x)
{
    return step(edge, x);
}

float3 mx_smoothstep(float3 edge0, float3 edge1, float3 x)
{
    return smoothstep(edge0, edge1, x);
}

float4 mx_sin(float4 angle)
{
    return sin(angle);
}

float4 mx_cos(float4 angle)
{
    return cos(angle);
}

float4 mx_tan(float4 angle)
{
    return tan(angle);
}

float4 mx_abs(float4 x)
{
    return abs(x);
}

float4 mx_sign(float4 x)
{
    return sign(x);
}

float4 mx_floor(float4 x)
{
    return floor(x);
}

float4 mx_ceil(float4 x)
{
    return ceil(x);
}

float4 mx_frac(float4 x)
{
    return frac(x);
}

float4 mx_mod(float4 x, float4 y)
{
    return fmod(x, y);
}

float4 mx_min(float4 a, float4 b)
{
    return min(a, b);
}

float4 mx_max(float4 a, float4 b)
{
    return max(a, b);
}

float4 mx_clamp(float4 x, float4 minVal, float4 maxVal)
{
    return saturate(clamp(x, minVal, maxVal));
}

float4 mx_mix(float4 a, float4 b, float4 t)
{
    return lerp(a, b, t);
}

float4 mx_mix(float4 a, float4 b, float t)
{
    return lerp(a, b, t);
}

float4 mx_step(float4 edge, float4 x)
{
    return step(edge, x);
}

float4 mx_smoothstep(float4 edge0, float4 edge1, float4 x)
{
    return smoothstep(edge0, edge1, x);
}

// ============================================================================
// Vector Operations
// ============================================================================

float mx_dot(float2 a, float2 b)
{
    return dot(a, b);
}

float mx_dot(float3 a, float3 b)
{
    return dot(a, b);
}

float mx_dot(float4 a, float4 b)
{
    return dot(a, b);
}

float mx_length(float2 v)
{
    return length(v);
}

float mx_length(float3 v)
{
    return length(v);
}

float mx_length(float4 v)
{
    return length(v);
}

float mx_distance(float2 a, float2 b)
{
    return distance(a, b);
}

float mx_distance(float3 a, float3 b)
{
    return distance(a, b);
}

float mx_distance(float4 a, float4 b)
{
    return distance(a, b);
}

float2 mx_normalize(float2 v)
{
    return normalize(v);
}

float3 mx_normalize(float3 v)
{
    return normalize(v);
}

float4 mx_normalize(float4 v)
{
    return normalize(v);
}

float3 mx_cross(float3 a, float3 b)
{
    return cross(a, b);
}

float2 mx_faceforward(float2 n, float2 i, float2 nref)
{
    return faceforward(n, i, nref);
}

float3 mx_faceforward(float3 n, float3 i, float3 nref)
{
    return faceforward(n, i, nref);
}

float4 mx_faceforward(float4 n, float4 i, float4 nref)
{
    return faceforward(n, i, nref);
}

float2 mx_reflect(float2 i, float2 n)
{
    return reflect(i, n);
}

float3 mx_reflect(float3 i, float3 n)
{
    return reflect(i, n);
}

float4 mx_reflect(float4 i, float4 n)
{
    return reflect(i, n);
}

float2 mx_refract(float2 i, float2 n, float eta)
{
    return refract(i, n, eta);
}

float3 mx_refract(float3 i, float3 n, float eta)
{
    return refract(i, n, eta);
}

float4 mx_refract(float4 i, float4 n, float eta)
{
    return refract(i, n, eta);
}

// ============================================================================
// Matrix Operations
// ============================================================================

float4x4 mx_transpose(float4x4 m)
{
    return transpose(m);
}

float4 mx_determinant(float4x4 m)
{
    return determinant(m);
}

float4x4 mx_inverse(float4x4 m)
{
    return inverse(m);
}

float3x3 mx_transpose(float3x3 m)
{
    return transpose(m);
}

float mx_determinant(float3x3 m)
{
    return determinant(m);
}

float3x3 mx_inverse(float3x3 m)
{
    return inverse(m);
}

// ============================================================================
// Color Functions
// ============================================================================

float3 mx_rgbtohsv(float3 c)
{
    float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    float4 p = lerp(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
    float4 q = lerp(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

float3 mx_hsvtorgb(float3 c)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float3 mx_srgb_encode(float3 color)
{
    // Simple sRGB gamma correction
    float3 linearColor = color;
    float3 result;
    result.r = linearColor.r <= 0.0031308 ? 12.92 * linearColor.r : 1.055 * pow(linearColor.r, 1.0 / 2.4) - 0.055;
    result.g = linearColor.g <= 0.0031308 ? 12.92 * linearColor.g : 1.055 * pow(linearColor.g, 1.0 / 2.4) - 0.055;
    result.b = linearColor.b <= 0.0031308 ? 12.92 * linearColor.b : 1.055 * pow(linearColor.b, 1.0 / 2.4) - 0.055;
    return result;
}

float3 mx_srgb_decode(float3 color)
{
    // Inverse sRGB gamma correction
    float3 sRGBColor = color;
    float3 linearColor;
    linearColor.r = sRGBColor.r <= 0.04045 ? sRGBColor.r / 12.92 : pow((sRGBColor.r + 0.055) / 1.055, 2.4);
    linearColor.g = sRGBColor.g <= 0.04045 ? sRGBColor.g / 12.92 : pow((sRGBColor.g + 0.055) / 1.055, 2.4);
    linearColor.b = sRGBColor.b <= 0.04045 ? sRGBColor.b / 12.92 : pow((sRGBColor.b + 0.055) / 1.055, 2.4);
    return linearColor;
}

// ============================================================================
// Noise Functions
// ============================================================================

float mx_random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

float mx_cellnoise2d(float2 uv)
{
    return mx_random(floor(uv));
}

float mx_cellnoise3d(float3 p)
{
    return mx_random(floor(p));
}

float mx_worleynoise2d(float2 uv)
{
    float2 i = floor(uv);
    float2 f = frac(uv);
    float minDist = 1.0;
    
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            float2 neighbor = float2(float(x), float(y));
            float2 point = float2(mx_random(i + neighbor), mx_random(i + neighbor + float2(0.5, 0.5)));
            float2 diff = neighbor + point - f;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    
    return minDist;
}

float mx_worleynoise3d(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);
    float minDist = 1.0;
    
    for (int z = -1; z <= 1; z++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {
                float3 neighbor = float3(float(x), float(y), float(z));
                float3 point = float3(
                    mx_random(i.xy + neighbor.xy + float2(0.5, 0.0)),
                    mx_random(i.xy + neighbor.xy + float2(0.0, 0.5)),
                    mx_random(i.xy + neighbor.xy + float2(0.5, 0.5))
                );
                float3 diff = neighbor + point - f;
                float dist = length(diff);
                minDist = min(minDist, dist);
            }
        }
    }
    
    return minDist;
}

float mx_fractalnoise2d(float2 uv, int octaves, float persistence)
{
    float total = 0.0;
    float frequency = 1.0;
    float amplitude = 1.0;
    float maxValue = 0.0;
    
    for (int i = 0; i < octaves; i++)
    {
        total += mx_worleynoise2d(uv * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0;
    }
    
    return total / maxValue;
}

float mx_fractalnoise3d(float3 p, int octaves, float persistence)
{
    float total = 0.0;
    float frequency = 1.0;
    float amplitude = 1.0;
    float maxValue = 0.0;
    
    for (int i = 0; i < octaves; i++)
    {
        total += mx_worleynoise3d(p * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0;
    }
    
    return total / maxValue;
}

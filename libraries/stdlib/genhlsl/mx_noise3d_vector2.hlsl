// Noise functions - 3D vector2 output
// Generated from MaterialX library

float2 mx_noise3d_vector2(float amplitude, float3 pivot, float3 freq, float3 offset)
{
    float3 uv = pivot * freq + offset;
    float2 noise = float2(
        mx_fractalnoise3d(uv),
        mx_fractalnoise3d(uv + float3(23.0, 17.0, 41.0)));
    return amplitude * noise;
}

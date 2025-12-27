// Noise functions - 2D vector3 output
// Generated from MaterialX library

float3 mx_noise2d_vector3(float amplitude, float2 pivot, float2 freq, float2 offset)
{
    float2 uv = pivot * freq + offset;
    float3 noise = float3(
        mx_fractalnoise2d(uv),
        mx_fractalnoise2d(uv + float2(23.0, 17.0)),
        mx_fractalnoise2d(uv + float2(47.0, 41.0)));
    return amplitude * noise;
}

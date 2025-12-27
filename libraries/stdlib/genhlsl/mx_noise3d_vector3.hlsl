// Noise functions - 3D vector3 output
// Generated from MaterialX library

float3 mx_noise3d_vector3(float amplitude, float3 pivot, float3 freq, float3 offset)
{
    float3 uv = pivot * freq + offset;
    float3 noise = float3(
        mx_fractalnoise3d(uv),
        mx_fractalnoise3d(uv + float3(23.0, 17.0, 41.0)),
        mx_fractalnoise3d(uv + float3(7.0, 13.0, 19.0)));
    return amplitude * noise;
}

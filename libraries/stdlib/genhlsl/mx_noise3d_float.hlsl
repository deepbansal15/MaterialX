// Noise functions - 3D float output
// Generated from MaterialX library

float mx_noise3d_float(float amplitude, float3 pivot, float3 freq, float3 offset)
{
    float3 uv = pivot * freq + offset;
    return amplitude * mx_fractalnoise3d(uv);
}

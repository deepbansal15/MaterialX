// Noise functions - 2D vector2 output
// Generated from MaterialX library

float2 mx_noise2d_vector2(float amplitude, float2 pivot, float2 freq, float2 offset)
{
    float2 uv = pivot * freq + offset;
    float2 noise = float2(mx_fractalnoise2d(uv), mx_fractalnoise2d(uv + float2(23.0, 17.0)));
    return amplitude * noise;
}

// Noise functions - 2D float output
// Generated from MaterialX library

float mx_noise2d_float(float amplitude, float2 pivot, float2 freq, float2 offset)
{
    float2 uv = pivot * freq + offset;
    return amplitude * mx_fractalnoise2d(uv);
}

// Noise functions - 3D vector4 output
// Generated from MaterialX library

float4 mx_noise3d_vector4(float amplitude, float4 pivot, float4 freq, float4 offset)
{
    float4 uv = pivot * freq + offset;
    float4 noise = float4(
        mx_fractalnoise3d(uv.xyz),
        mx_fractalnoise3d(uv.xyz + float3(23.0, 17.0, 41.0)),
        mx_fractalnoise3d(uv.xyz + float3(7.0, 13.0, 19.0)),
        mx_fractalnoise3d(uv.xyz + float3(11.0, 3.0, 31.0)));
    return amplitude * noise;
}

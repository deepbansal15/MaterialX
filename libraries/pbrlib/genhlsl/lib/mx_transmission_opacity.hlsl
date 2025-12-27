// HLSL Transmission/Opacity for MaterialX

float3 mx_get_transmission(float3 transmissionColor, float transmissionAmount, float thickness, float ior)
{
    return transmissionColor * transmissionAmount;
}

float mx_get_opacity(float3 transmissionColor, float transmissionAmount)
{
    return 1.0 - transmissionAmount * (1.0 - dot(transmissionColor, float3(0.3333, 0.3334, 0.3333)));
}

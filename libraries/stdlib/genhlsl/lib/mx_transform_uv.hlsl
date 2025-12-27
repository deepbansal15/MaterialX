// HLSL UV Transform Functions for MaterialX

float2 mx_transform_uv(float2 uv, float2 scale, float2 offset, float rotation)
{
    // Apply scale
    uv = uv * scale;
    
    // Apply rotation
    float s = sin(rotation);
    float c = cos(rotation);
    float2 rotatedUV;
    rotatedUV.x = uv.x * c - uv.y * s;
    rotatedUV.y = uv.x * s + uv.y * c;
    uv = rotatedUV;
    
    // Apply offset
    uv = uv + offset;
    
    return uv;
}

float2 mx_transform_uv_vflip(float2 uv, float2 scale, float2 offset, float rotation)
{
    // Apply vertical flip first
    uv.y = 1.0 - uv.y;
    
    // Then apply regular transform
    return mx_transform_uv(uv, scale, offset, rotation);
}

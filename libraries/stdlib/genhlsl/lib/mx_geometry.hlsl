// HLSL Geometry Functions for MaterialX

float3 mx_faceforward(float3 n, float3 i, float3 nref)
{
    return faceforward(n, i, nref);
}

float3 mx_reflect(float3 i, float3 n)
{
    return reflect(i, n);
}

float3 mx_refract(float3 i, float3 n, float eta)
{
    return refract(i, n, eta);
}

float3 mx_transformpoint(float3 position, matrix transform)
{
    return mul(float4(position, 1.0), transform).xyz;
}

float3 mx_transformvector(float3 vector, matrix transform)
{
    return mul(vector, (float3x3)transform);
}

float3 mx_transformnormal(float3 normal, matrix transform)
{
    float3 transformedNormal = mul(normal, (float3x3)transform);
    return normalize(transformedNormal);
}

float3 mx_getpositionobject(float3 position)
{
    return position;
}

float3 mx_getnormalworld(float3 normal, matrix worldInverseTranspose)
{
    return normalize(mul(normal, (float3x3)worldInverseTranspose));
}

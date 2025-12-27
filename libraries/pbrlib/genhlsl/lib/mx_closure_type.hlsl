//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// NOTE: In HLSL, BSDF, EDF, and VDF types are defined as structs.
// The actual struct definitions are generated in the shader code.

#ifndef MX_CLOSURE_TYPE_HLSL
#define MX_CLOSURE_TYPE_HLSL

// Closure type identifiers
#define MX_CLOSURE_TYPE_NONE              0
#define MX_CLOSURE_TYPEBSDF               1
#define MX_CLOSURE_TYPEEDF                2
#define MX_CLOSURE_TYPEVDF                3

// BSDF types
#define MX_BSDF_NONE                      0
#define MX_BSDF_DIFFUSE_REFLECTION        1
#define MX_BSDF_DIFFUSE_TRANSMISSION      2
#define MX_BSDF_SPECULAR_REFLECTION       3
#define MX_BSDF_SPECULAR_TRANSMISSION     4
#define MX_BSDF_MEASURED                  5

// EDF types
#define MX_EDF_NONE                       0
#define MX_EDF_DIFFUSE                    1
#define MX_EDF_MEASURED                   2

// VDF types
#define MX_VDF_NONE                       0
#define MX_VDF_ABSORPTION                 1

// Directional albedo table size
#define MX_DIRECTIONAL_ALBEDO_TABLE_SIZE  128

#endif

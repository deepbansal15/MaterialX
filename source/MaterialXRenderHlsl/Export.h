//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_RENDERHLSL_EXPORT_H
#define MATERIALX_RENDERHLSL_EXPORT_H

#include <MaterialXRender/Export.h>

#ifdef MATERIALX_RENDERHLSL_EXPORTS
#define MX_RENDERHLSL_API MATERIALX_SYMBOL_EXPORT
#else
#define MX_RENDERHLSL_API MATERIALX_SYMBOL_IMPORT
#endif

#endif

#
# Copyright Contributors to the MaterialX Project
# SPDX-License-Identifier: Apache-2.0
#

# MaterialXRenderHlsl only builds on Windows due to DirectX dependencies
if(NOT WIN32)
    return()
endif()

# Find DirectX dependencies
find_package(directxheaders CONFIG REQUIRED)
find_package(Microsoft.DirectX.Agility.SDK CONFIG REQUIRED)

set(MATERIALX_RENDERHLSL_HEADERS
    Export.h
    Dx12Renderer.h
    Dx12Framebuffer.h
    Dx12State.h
    Dx12PipelineStateObject.h
    HlslMaterial.h
    HlslProgram.h
    Dx12TextureHandler.h
    TextureBakerHlsl.h
)

set(MATERIALX_RENDERHLSL_SOURCES
    Dx12Renderer.cpp
    Dx12Framebuffer.cpp
    Dx12State.cpp
    Dx12PipelineStateObject.cpp
    HlslMaterial.cpp
    HlslProgram.cpp
    Dx12TextureHandler.cpp
    TextureBakerHlsl.cpp
)

add_library(MaterialXRenderHlsl
    ${MATERIALX_RENDERHLSL_HEADERS}
    ${MATERIALX_RENDERHLSL_SOURCES}
)

target_link_libraries(MaterialXRenderHlsl
    PUBLIC
        MaterialXRender
        MaterialXGenHlsl
        directxheaders::directxheaders
        Microsoft::DirectX.Agility.SDK
)

target_include_directories(MaterialXRenderHlsl
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../MaterialXRender
        ${CMAKE_CURRENT_SOURCE_DIR}/../MaterialXGenHlsl
)

if(MATERIALX_BUILD_SHARED_LIBS)
    target_compile_definitions(MaterialXRenderHlsl PRIVATE MATERIALX_RENDERHLSL_EXPORTS)
endif()

set_target_properties(MaterialXRenderHlsl PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    FOLDER "Libraries"
)

install(TARGETS MaterialXRenderHlsl
    EXPORT MaterialX
    RUNTIME DESTINATION ${MATERIALX_INSTALL_BIN_PATH}
    LIBRARY DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
    ARCHIVE DESTINATION ${MATERIALX_INSTALL_LIB_PATH}
)

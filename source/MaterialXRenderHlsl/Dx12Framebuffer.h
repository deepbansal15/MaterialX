//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_DX12FRAMEBUFFER_H
#define MATERIALX_DX12FRAMEBUFFER_H

#include <MaterialXRenderHlsl/Export.h>

#include <MaterialXRender/ImageHandler.h>

#include <d3d12.h>
#include <wrl.h>

MATERIALX_NAMESPACE_BEGIN

class Dx12Framebuffer;

/// Shared pointer to a Dx12Framebuffer
using Dx12FramebufferPtr = std::shared_ptr<Dx12Framebuffer>;

/// @class Dx12Framebuffer
/// Wrapper for a DirectX 12 framebuffer (render target and depth stencil)
class MX_RENDERHLSL_API Dx12Framebuffer
{
  public:
    /// Create a new framebuffer
    static Dx12FramebufferPtr create(ID3D12Device* device,
                                    unsigned int width, unsigned int height,
                                    unsigned int channelCount,
                                    Image::BaseType baseType,
                                    ID3D12Resource* colorTexture = nullptr,
                                    bool encodeSrgb = false,
                                    DXGI_FORMAT pixelFormat = DXGI_FORMAT_UNKNOWN);

    /// Destructor
    virtual ~Dx12Framebuffer();

    /// Resize the framebuffer
    void resize(unsigned int width, unsigned int height, bool forceRecreate = false,
                DXGI_FORMAT pixelFormat = DXGI_FORMAT_UNKNOWN,
                ID3D12Resource* extColorTexture = nullptr);

    /// Set the encode sRGB flag
    void setEncodeSrgb(bool encode)
    {
        if (encode != _encodeSrgb)
        {
            _encodeSrgb = encode;
            resize(_width, _height, true);
        }
    }

    /// Return the encode sRGB flag
    bool getEncodeSrgb() const { return _encodeSrgb; }

    /// Return the framebuffer width
    unsigned int getWidth() const { return _width; }

    /// Return the framebuffer height
    unsigned int getHeight() const { return _height; }

    /// Return the color texture handle
    ID3D12Resource* getColorTexture() const { return _colorTexture.Get(); }

    /// Return the depth stencil texture handle
    ID3D12Resource* getDepthTexture() const { return _depthTexture.Get(); }

    /// Return the color texture CPU descriptor handle
    D3D12_CPU_DESCRIPTOR_HANDLE getColorCPUDescriptor() const { return _colorCPUDescriptor; }

    /// Return the depth stencil CPU descriptor handle
    D3D12_CPU_DESCRIPTOR_HANDLE getDepthCPUDescriptor() const { return _depthCPUDescriptor; }

    /// Return the color data of this framebuffer as an image
    ImagePtr getColorImage(ID3D12CommandQueue* commandQueue, ImagePtr image = nullptr);

  protected:
    Dx12Framebuffer(ID3D12Device* device,
                    unsigned int width, unsigned int height,
                    unsigned int channelCount,
                    Image::BaseType baseType,
                    ID3D12Resource* colorTexture = nullptr,
                    bool encodeSrgb = false,
                    DXGI_FORMAT pixelFormat = DXGI_FORMAT_UNKNOWN);

    /// Map image properties to DXGI format
    DXGI_FORMAT mapFormatToDXGI(Image::BaseType baseType, unsigned int channelCount, bool encodeSrgb);

  protected:
    unsigned int _width;
    unsigned int _height;
    unsigned int _channelCount;
    Image::BaseType _baseType;
    bool _encodeSrgb;

    Microsoft::WRL::ComPtr<ID3D12Device> _device;
    Microsoft::WRL::ComPtr<ID3D12Resource> _colorTexture;
    Microsoft::WRL::ComPtr<ID3D12Resource> _depthTexture;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsvHeap;

    D3D12_CPU_DESCRIPTOR_HANDLE _colorCPUDescriptor;
    D3D12_CPU_DESCRIPTOR_HANDLE _depthCPUDescriptor;

    bool _colorTextureOwned;
};

MATERIALX_NAMESPACE_END

#endif

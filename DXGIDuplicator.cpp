// DXGIDuplicator.cpp
#define NOMINMAX
#include <Windows.h>
#include <algorithm>
#include <mutex>

#include "DXGIDuplicator.h"

std::mutex capture_mutex_;

// ���캯��������ʲôҲ����
DXGIDuplicator::DXGIDuplicator()
{
}

// �����������ͷ���ض���
DXGIDuplicator::~DXGIDuplicator()
{
    if (duplication_)
    {
        duplication_->Release();
    }
    if (device_)
    {
        device_->Release();
    }
    if (deviceContext_)
    {
        deviceContext_->Release();
    }
}

bool DXGIDuplicator::InitD3D11Device()
{
    D3D_DRIVER_TYPE DriverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

    D3D_FEATURE_LEVEL FeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_1
    };
    UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);
    D3D_FEATURE_LEVEL FeatureLevel;

    for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
    {
        HRESULT hr = D3D11CreateDevice(
            nullptr,
            DriverTypes[DriverTypeIndex],
            nullptr, 0,
            FeatureLevels,
            NumFeatureLevels,
            D3D11_SDK_VERSION,
            &device_,
            &FeatureLevel,
            &deviceContext_);
        if (SUCCEEDED(hr))
        {
            break;
        }
    }

    if (device_ == nullptr || deviceContext_ == nullptr)
    {
        return false;
    }

    return true;
}

bool DXGIDuplicator::InitDuplication()
{
    HRESULT hr = S_OK;

    IDXGIDevice* dxgiDevice = nullptr;
    hr = device_->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
    if (FAILED(hr))
    {
        return false;
    }

    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    dxgiDevice->Release();
    if (FAILED(hr))
    {
        return false;
    }

    UINT output = 0;
    IDXGIOutput* dxgiOutput = nullptr;
    while (true)
    {
        hr = dxgiAdapter->EnumOutputs(output++, &dxgiOutput);
        if (hr == DXGI_ERROR_NOT_FOUND)
        {
            return false;
        }
        else
        {
            DXGI_OUTPUT_DESC desc;
            dxgiOutput->GetDesc(&desc);
            int width = desc.DesktopCoordinates.right - desc.DesktopCoordinates.left;
            int height = desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top;
            break;
        }
    }
    dxgiAdapter->Release();

    IDXGIOutput1* dxgiOutput1 = nullptr;
    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&dxgiOutput1));
    dxgiOutput->Release();
    if (FAILED(hr))
    {
        return false;
    }

    hr = dxgiOutput1->DuplicateOutput(device_, &duplication_);
    dxgiOutput1->Release();
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}



bool DXGIDuplicator::GetDesktopFrame(ID3D11Texture2D** texture)
{
    HRESULT hr = S_OK;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    IDXGIResource* resource = nullptr;
    ID3D11Texture2D* acquireFrame = nullptr;

    hr = duplication_->AcquireNextFrame(0, &frameInfo, &resource);
    if (FAILED(hr))
    {
        if (hr == DXGI_ERROR_WAIT_TIMEOUT)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&acquireFrame));
    resource->Release();
    if (FAILED(hr))
    {
        return false;
    }

    D3D11_TEXTURE2D_DESC desc;
    acquireFrame->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    device_->CreateTexture2D(&desc, NULL, texture);
    if (texture && *texture)
    {
        deviceContext_->CopyResource(*texture, acquireFrame);
    }
    acquireFrame->Release();

    hr = duplication_->ReleaseFrame();
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

bool DXGIDuplicator::GetDesktopFrameRegion(int left, int top, int width, int height,
    uint8_t** outData, int* outStride, int* outWidth, int* outHeight, int timeout_ms)
{
    if (!outData || !outStride || !outWidth || !outHeight)
        return false;

    HRESULT hr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    IDXGIResource* resource = nullptr;
    ID3D11Texture2D* acquireFrame = nullptr;

    // ��̬��������֡�����ٻ���ü�����
    static uint8_t* cachedFullBuffer = nullptr;
    static int cachedFullStride = 0;
    static int cachedFullWidth = 0;
    static int cachedFullHeight = 0;

    static bool firstFrameObtained = false;

    const int maxRetries = 500;
    int retries = 0;

    while (retries < maxRetries)
    {
        hr = duplication_->AcquireNextFrame(timeout_ms, &frameInfo, &resource);

        if (hr == DXGI_ERROR_WAIT_TIMEOUT)
        {
            // ʹ�û��������֡�ü�����
            if (cachedFullBuffer)
            {
                int outW = width;
                int outH = height;
                int outX = std::max(0, std::min(left, cachedFullWidth - 1));
                int outY = std::max(0, std::min(top, cachedFullHeight - 1));
                if (outX + outW > cachedFullWidth) outW = cachedFullWidth - outX;
                if (outY + outH > cachedFullHeight) outH = cachedFullHeight - outY;

                int rowSize = outW * 4;
                uint8_t* buffer = (uint8_t*)malloc(outH * rowSize);
                if (!buffer) return false;

                for (int y = 0; y < outH; ++y)
                {
                    memcpy(buffer + y * rowSize,
                        cachedFullBuffer + (outY + y) * cachedFullStride + outX * 4,
                        rowSize);
                }

                *outData = buffer;
                *outStride = rowSize;
                *outWidth = outW;
                *outHeight = outH;
                return true;
            }
            else
            {
                retries++;
                Sleep(1);
                continue;
            }
        }
        else if (FAILED(hr))
        {
            return false;
        }

        // �ɹ������֡��Դ
        hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&acquireFrame);
        resource->Release();
        if (FAILED(hr)) {
            duplication_->ReleaseFrame();
            return false;
        }

        D3D11_TEXTURE2D_DESC fullDesc;
        acquireFrame->GetDesc(&fullDesc);
        fullDesc.Usage = D3D11_USAGE_STAGING;
        fullDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        fullDesc.BindFlags = 0;
        fullDesc.MiscFlags = 0;

       /* if (fullDesc.Width < 100 || fullDesc.Height < 100)
        {
            printf("[!] ��ȡ֡�ߴ��쳣: %d x %d������\n", fullDesc.Width, fullDesc.Height);
            acquireFrame->Release();
            duplication_->ReleaseFrame();
            retries++;
            Sleep(5);
            continue;
        }*/

        ID3D11Texture2D* stagingTex = nullptr;
        hr = device_->CreateTexture2D(&fullDesc, nullptr, &stagingTex);
        if (FAILED(hr)) {
            acquireFrame->Release();
            duplication_->ReleaseFrame();
            return false;
        }

        deviceContext_->CopyResource(stagingTex, acquireFrame);

        D3D11_MAPPED_SUBRESOURCE mapped;
        hr = deviceContext_->Map(stagingTex, 0, D3D11_MAP_READ, 0, &mapped);
        if (FAILED(hr)) {
            stagingTex->Release();
            acquireFrame->Release();
            duplication_->ReleaseFrame();
            return false;
        }

        // ������֡���ݻ���
        int fullRowSize = fullDesc.Width * 4;
        int fullSize = fullRowSize * fullDesc.Height;
        if (cachedFullBuffer) free(cachedFullBuffer);
        cachedFullBuffer = (uint8_t*)malloc(fullSize);
        if (!cachedFullBuffer) {
            deviceContext_->Unmap(stagingTex, 0);
            stagingTex->Release();
            acquireFrame->Release();
            duplication_->ReleaseFrame();
            return false;
        }

        for (int y = 0; y < (int)fullDesc.Height; ++y)
        {
            memcpy(cachedFullBuffer + y * fullRowSize,
                (uint8_t*)mapped.pData + y * mapped.RowPitch,
                fullRowSize);
        }
        cachedFullStride = fullRowSize;
        cachedFullWidth = fullDesc.Width;
        cachedFullHeight = fullDesc.Height;

        // ͬʱ�ü���������
        int outX = std::max(0, std::min(left, static_cast<int>(fullDesc.Width) - 1));
        int outY = std::max(0, std::min(top, static_cast<int>(fullDesc.Height) - 1));
        int outW = std::min(width, (int)(fullDesc.Width - outX));
        int outH = std::min(height, (int)(fullDesc.Height - outY));
        int rowSize = outW * 4;

        uint8_t* buffer = (uint8_t*)malloc(outH * rowSize);
        if (!buffer) {
            deviceContext_->Unmap(stagingTex, 0);
            stagingTex->Release();
            acquireFrame->Release();
            duplication_->ReleaseFrame();
            return false;
        }

        for (int y = 0; y < outH; ++y)
        {
            memcpy(buffer + y * rowSize,
                (uint8_t*)mapped.pData + (outY + y) * mapped.RowPitch + outX * 4,
                rowSize);
        }

        deviceContext_->Unmap(stagingTex, 0);
        stagingTex->Release();
        acquireFrame->Release();
        duplication_->ReleaseFrame();

        // ��֤�������ǿ�
        bool bufferIsValid = false;
        for (int i = 0; i < outH * rowSize; ++i)
        {
            if (buffer[i] != 0) {
                bufferIsValid = true;
                break;
            }
        }

        if (!firstFrameObtained)
        {
           /* if (!bufferIsValid)
            {
                free(buffer);
                retries++;
                Sleep(5);
                continue;
            }
            else
            {*/
                firstFrameObtained = true;
            //}
        }

        if (bufferIsValid)
        {
            *outData = buffer;
            *outStride = rowSize;
            *outWidth = outW;
            *outHeight = outH;
            return true;
        }
        else
        {
            free(buffer);
            retries++;
            Sleep(5);
            continue;
        }
    }

    return false;
}






//bool DXGIDuplicator::GetDesktopFrameRegion(int left, int top, int width, int height,
//    uint8_t** outData, int* outStride, int* outWidth, int* outHeight)
//{
//    // ��ȡ����֡����
//    ID3D11Texture2D* texture = nullptr;
//    if (!GetDesktopFrame(&texture) || !texture)
//        return false;
//
//    // ��ȡ����������Ϣ
//    D3D11_TEXTURE2D_DESC desc;
//    texture->GetDesc(&desc);
//
//    // ӳ���������ݹ�CPU��ȡ
//    D3D11_MAPPED_SUBRESOURCE mapped;
//    if (FAILED(deviceContext_->Map(texture, 0, D3D11_MAP_READ, 0, &mapped)))
//    {
//        texture->Release();
//        return false;
//    }
//
//    int bpp = 4;  // ÿ����4�ֽ� BGRA
//    int fullWidth = desc.Width;
//    int fullHeight = desc.Height;
//    int rowPitch = mapped.RowPitch;
//
//    // �ж����������Ƿ�Խ��
//    if (left < 0 || top < 0 || left + width > fullWidth || top + height > fullHeight)
//    {
//        deviceContext_->Unmap(texture, 0);
//        texture->Release();
//        return false;
//    }
//
//    // ��������ͼ���ڴ棨�ж���=width * 4��
//    int regionStride = width * bpp;
//    uint8_t* buffer = (uint8_t*)malloc(regionStride * height);
//    if (!buffer)
//    {
//        deviceContext_->Unmap(texture, 0);
//        texture->Release();
//        return false;
//    }
//
//    // ����ÿһ����������
//    uint8_t* src = static_cast<uint8_t*>(mapped.pData);
//    for (int y = 0; y < height; ++y)
//    {
//        memcpy(
//            buffer + y * regionStride,
//            src + (top + y) * rowPitch + left * bpp,
//            regionStride
//        );
//    }
//
//    // ȡ��ӳ�䲢�ͷ�����
//    deviceContext_->Unmap(texture, 0);
//    texture->Release();
//
//    // ��������
//    *outData = buffer;
//    *outStride = regionStride;
//    *outWidth = width;
//    *outHeight = height;
//    return true;
//}

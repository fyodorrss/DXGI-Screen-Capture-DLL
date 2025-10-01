// DXGIDuplicator.h

#include <d3d11.h>
#include <dxgi1_2.h>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

class DXGIDuplicator
{
public:
    DXGIDuplicator();
    ~DXGIDuplicator();

    bool InitD3D11Device();

    bool InitDuplication();

    bool GetDesktopFrame(ID3D11Texture2D** texture);

    // 新增区域截图，返回 CPU 可读数据
    bool GetDesktopFrameRegion(int left, int top, int width, int height,
        uint8_t** outData, int* outStride, int* outWidth, int* outHeight, int timeout_ms);

private:
    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* deviceContext_ = nullptr;
    IDXGIOutputDuplication* duplication_ = nullptr;
};
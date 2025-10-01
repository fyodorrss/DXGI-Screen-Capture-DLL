#include "DXGIDLLExport.h"
#include "DXGIDuplicator.h"
#include <string>

static DXGIDuplicator* duplicator = nullptr;

bool DXGIDLL_CALL InitDuplicator()
{
    duplicator = new DXGIDuplicator();
    if (!duplicator->InitD3D11Device()) return false;
    if (!duplicator->InitDuplication()) return false;
    return true;
}

void DXGIDLL_CALL FreeDuplicator()
{
    if (duplicator)
    {
        delete duplicator;
        duplicator = nullptr;
    }
}

bool DXGIDLL_CALL GetDesktopFrameRegion(int left, int top, int width, int height,
    uint8_t** outData, int* outStride, int* outWidth, int* outHeight, int timeout_ms)
{
    if (!duplicator) return false;
    return duplicator->GetDesktopFrameRegion(left, top, width, height, outData, outStride, outWidth, outHeight, timeout_ms);
}

void DXGIDLL_CALL FreeBuffer(uint8_t* buffer)
{
    if (buffer)
    {
        free(buffer);
    }
}


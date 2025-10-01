#pragma once
#include <stdint.h>

#ifdef DXGIDLL_EXPORTS
#define DXGIDLL_API __declspec(dllexport)
#else
#define DXGIDLL_API __declspec(dllimport)
#endif

#define DXGIDLL_CALL __stdcall

#ifdef __cplusplus
extern "C" {
#endif

    DXGIDLL_API bool DXGIDLL_CALL InitDuplicator();
    DXGIDLL_API void DXGIDLL_CALL FreeDuplicator();

    DXGIDLL_API bool DXGIDLL_CALL GetDesktopFrameRegion(
        int left, int top, int width, int height,
        uint8_t** outData, int* outStride, int* outWidth, int* outHeight, int timeout_ms);

    DXGIDLL_API void DXGIDLL_CALL FreeBuffer(uint8_t* buffer);

#ifdef __cplusplus
}
#endif

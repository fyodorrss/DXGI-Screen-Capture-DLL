import ctypes
import os
import sys
import threading
from ctypes import *
import numpy as np

def get_resource_path(relative_path):
    base_dir = getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__)))
    return os.path.join(base_dir, relative_path)

dll_path = get_resource_path("DXGI-Screen-Capture-DLL.dll")
dxgi = ctypes.CDLL(dll_path)

dxgi.InitDuplicator.restype = c_bool
dxgi.FreeDuplicator.restype = None

dxgi.GetDesktopFrameRegion.argtypes = [
    c_int, c_int, c_int, c_int,
    POINTER(POINTER(c_uint8)),
    POINTER(c_int), POINTER(c_int), POINTER(c_int),
    c_int  
]

dxgi.GetDesktopFrameRegion.restype = c_bool

dxgi.FreeBuffer.argtypes = [POINTER(c_uint8)]
dxgi.FreeBuffer.restype = None

dxgi_lock = threading.Lock()

if not dxgi.InitDuplicator():
    print("init DXGI failed")
    exit(1)

class DXGIScreenshot:
    def __init__(self, width, height, stride, pixels):
        self.width = width
        self.height = height
        self.stride = stride
        self.pixels = pixels

    def __array__(self, dtype=None):
        arr = np.frombuffer(self.pixels, dtype=np.uint8)
        arr = arr[:self.height * self.stride]
        arr = arr.reshape((self.height, self.stride // 4, 4))
        arr = arr[:, :self.width, :] 
        return arr

def grab_region(region: dict, timeout_ms=0):
    required_keys = {"left", "top", "width", "height"}
    if not isinstance(region, dict) or not required_keys.issubset(region.keys()):
        raise ValueError("region must be a dict with keys: 'left', 'top', 'width', 'height'")

    left = int(region["left"])
    top = int(region["top"])
    width = int(region["width"])
    height = int(region["height"])

    buffer_ptr = POINTER(c_uint8)()
    stride = c_int()
    out_width = c_int()
    out_height = c_int()

    with dxgi_lock:
        success = dxgi.GetDesktopFrameRegion(
            left, top, width, height,
            byref(buffer_ptr), byref(stride),
            byref(out_width), byref(out_height),
            timeout_ms
        )

        if not success or not buffer_ptr:
            return None

        size_bytes = stride.value * out_height.value
        raw_data = ctypes.string_at(buffer_ptr, size_bytes)
        dxgi.FreeBuffer(buffer_ptr)

    return DXGIScreenshot(out_width.value, out_height.value, stride.value, raw_data)

import cv2
import numpy as np
import time
from datetime import datetime

def test_screenshot_speed_and_save(region: dict, repeat=50, folder="screenshots"):
    """
    测试 grab_region 截图速度，并将每次截图保存到指定文件夹
    
    region: 截图区域 dict，包含 left, top, width, height
    repeat: 循环次数
    folder: 保存截图的文件夹
    """
    os.makedirs(folder, exist_ok=True)  # 创建文件夹
    times = []

    for i in range(repeat):
        start = time.time()
        screenshot = grab_region(region)
        end = time.time()

        if screenshot is None:
            print(f"第 {i+1} 次截图失败")
            continue

        elapsed_ms = (end - start) * 1000
        times.append(elapsed_ms)

        # 转成 numpy 数组
        img = np.array(screenshot)
        img = cv2.cvtColor(img, cv2.COLOR_BGRA2BGR)

        # 文件名带时间戳
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        filename = f"screenshot_{timestamp}.png"
        file_path = os.path.join(folder, filename)
        cv2.imwrite(file_path, img)

        print(f"第 {i+1} 次截图耗时: {elapsed_ms:.2f} ms, 保存到: {file_path}")

    if times:
        avg_time = sum(times) / len(times)
        print(f"{len(times)} 次截图平均耗时: {avg_time:.2f} ms")
        return avg_time
    else:
        print("没有成功截图，无法计算平均耗时")
        return None

# 示例使用
if __name__ == "__main__":
    test_region = {"left": 0, "top": 0, "width": 800, "height": 600}
    test_screenshot_speed_and_save(test_region, repeat=10, folder="test_screenshots")




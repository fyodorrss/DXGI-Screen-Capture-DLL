# DXGI 屏幕捕获工具

## 项目概述

基于 Windows DXGI 的高效屏幕截图工具，支持多线程安全和性能测试。适用于实时监控、自动化测试等场景。

## 特性

- 高效区域截图，支持自定义坐标和尺寸。

## 性能测试
```python
test_region = {"left": 0, "top": 0, "width": 800, "height": 600}
avg_time = test_screenshot_speed_and_save(test_region, repeat=10, folder="test_screenshots")
```

- **参数**：`region`, `repeat`（默认 50）, `folder`（默认 `screenshots`）。
- **输出**：打印耗时和路径，返回平均耗时。

## 示例
```python
if __name__ == "__main__":
    test_region = {"left": 0, "top": 0, "width": 800, "height": 600}
    test_screenshot_speed_and_save(test_region, repeat=10, folder="test_screenshots")
```

**输出示例**：
```
第 1 次截图耗时: 4.96 ms, 保存到: test_screenshots\screenshot_20251001_194704_676126.png
第 2 次截图耗时: 1.31 ms, 保存到: test_screenshots\screenshot_20251001_194704_684350.png
第 3 次截图耗时: 1.10 ms, 保存到: test_screenshots\screenshot_20251001_194704_691891.png
第 4 次截图耗时: 1.43 ms, 保存到: test_screenshots\screenshot_20251001_194704_700142.png
第 5 次截图耗时: 5.28 ms, 保存到: test_screenshots\screenshot_20251001_194704_712430.png
第 6 次截图耗时: 4.47 ms, 保存到: test_screenshots\screenshot_20251001_194704_723550.png
第 7 次截图耗时: 1.62 ms, 保存到: test_screenshots\screenshot_20251001_194704_732248.png
第 8 次截图耗时: 4.55 ms, 保存到: test_screenshots\screenshot_20251001_194704_743717.png
第 9 次截图耗时: 4.48 ms, 保存到: test_screenshots\screenshot_20251001_194704_755953.png
第 10 次截图耗时: 2.01 ms, 保存到: test_screenshots\screenshot_20251001_194704_765255.png
10 次截图平均耗时: 3.12 ms
```

## 贡献与反馈

通过 GitHub Issues 或 Pull Requests 提交问题或建议，提供系统环境、日志和重现步骤。

## 许可证

MIT 许可证（详见 LICENSE）。DLL 版权归 mono 所有，遵守其许可协议。
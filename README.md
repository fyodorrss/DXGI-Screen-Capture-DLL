**[[中文]][readme_zh]**

# DXGI Screen Capture Tool

## Overview

A high-efficiency screen capture tool based on Windows DXGI, supporting thread-safe operations and performance testing. Ideal for real-time monitoring and automated testing.

## Features

- Efficient region capture with customizable coordinates and dimensions.

## Performance Testing
```python
test_region = {"left": 0, "top": 0, "width": 800, "height": 600}
avg_time = test_screenshot_speed_and_save(test_region, repeat=10, folder="test_screenshots")
```

- **Parameters**: `region`, `repeat` (default 50), `folder` (default `screenshots`).
- **Output**: Prints capture duration and file paths, returns average time.

## Example
```python
if __name__ == "__main__":
    test_region = {"left": 0, "top": 0, "width": 800, "height": 600}
    test_screenshot_speed_and_save(test_region, repeat=10, folder="test_screenshots")
```

**Sample Output**:
```
Capture 1 time: 4.96 ms, saved to: test_screenshots\screenshot_20251001_194704_676126.png
Capture 2 time: 1.31 ms, saved to: test_screenshots\screenshot_20251001_194704_684350.png
Capture 3 time: 1.10 ms, saved to: test_screenshots\screenshot_20251001_194704_691891.png
Capture 4 time: 1.43 ms, saved to: test_screenshots\screenshot_20251001_194704_700142.png
Capture 5 time: 5.28 ms, saved to: test_screenshots\screenshot_20251001_194704_712430.png
Capture 6 time: 4.47 ms, saved to: test_screenshots\screenshot_20251001_194704_723550.png
Capture 7 time: 1.62 ms, saved to: test_screenshots\screenshot_20251001_194704_732248.png
Capture 8 time: 4.55 ms, saved to: test_screenshots\screenshot_20251001_194704_743717.png
Capture 9 time: 4.48 ms, saved to: test_screenshots\screenshot_20251001_194704_755953.png
Capture 10 time: 2.01 ms, saved to: test_screenshots\screenshot_20251001_194704_765255.png
Average time for 10 captures: 3.12 ms
```

## Contribution and Feedback

Submit issues or pull requests via GitHub, providing system details, logs, and reproduction steps.

## License

MIT License (see LICENSE file). DLL copyright belongs to mono; adhere to its licensing terms.

[readme_zh]: /docs/zh/README.md
[readme_en]: /docs/en/README.md

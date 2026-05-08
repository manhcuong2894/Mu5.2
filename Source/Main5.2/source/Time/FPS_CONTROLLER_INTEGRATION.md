# FPS Control System Integration

## Overview
A comprehensive FPS (Frames Per Second) control system has been integrated into your MU Online project to support frame rate limiting, FPS override functionality, and both original and optimized pipeline support.

## Files Created

### 1. `source\Time\CFPSController.h`
Header file containing:
- **CFPSController class**: Singleton for managing FPS control
- **Key Methods**:
  - `BeginFrame()`: Mark the start of a frame
  - `EndFrame()`: Mark the end of frame and apply limiting
  - `SetTargetFPS(fps)`: Set target frame rate
  - `SetFPSOverride(active, overrideFps)`: Enable/disable FPS override
  - `GetCurrentFPS()`: Get actual FPS being achieved
  - `GetFrameTime()`: Get frame time in milliseconds

- **Global Variables**:
  - `g_FPSOverrideActive`: Flag to enable FPS override (0 or 1)
  - `g_FPSOverrideMs`: Override frame time in milliseconds
  - `g_UseOriginal150FPSPipeline`: Toggle between original and optimized pipeline

- **Macro**: `gFPSController` for easy singleton access

### 2. `source\Time\CFPSController.cpp`
Implementation file with:
- Complete frame timing using high-resolution clock
- Smart frame limiting with two pipelines:
  - **Original Pipeline**: Simple sleep-based limiting
  - **Optimized Pipeline**: Advanced sleep/spin mechanism with:
    - Adaptive sleeping to avoid busy-waiting
    - Spin waiting for precise timing
    - Yield to reduce CPU usage when needed
- FPS override support allowing dynamic FPS changes at runtime
- Proper initialization and shutdown

## Files Modified

### `source\WINHANDLE.cpp`
Changes:
- Added `#include "Time/CFPSController.h"` 
- Initialize CFPSController in `winLoop()` before the main loop
- Call `gFPSController->BeginFrame()` at the start of each frame
- Call `gFPSController->EndFrame()` at the end of rendering (after Scene() call)
- Shutdown controller after main loop exits

## Integration Details

### Frame Flow
```
while (game is running) {
    gFPSController->BeginFrame()           // Start timing
    
    [Process Windows Messages]
    [Call Scene(g_hDC) for rendering]     // Main render call
    
    gFPSController->EndFrame()             // Apply frame limiting
    [Protocol updates]
}
```

### FPS Override Feature
The system supports dynamic FPS override:
```cpp
// Enable 60 FPS override
gFPSController->SetFPSOverride(true, 60.0);

// Or use global variables directly
g_FPSOverrideActive = 1;
g_FPSOverrideMs = 16.666;  // ~60 FPS
```

### Pipeline Support
Two frame limiting strategies:
- **Original (g_UseOriginal150FPSPipeline = true)**: Basic Sleep() call
- **Optimized (g_UseOriginal150FPSPipeline = false)**: Hybrid sleep/spin for better precision

## Technical Features

1. **High-Resolution Timing**: Uses `std::chrono::high_resolution_clock` for accurate measurements
2. **CPU-Efficient**: Implements sleep/spin strategy to avoid busy-waiting
3. **Adaptive**: Automatically switches between sleep and spin based on remaining time
4. **Thread-Safe Singleton**: Proper singleton pattern with lazy initialization
5. **Extensible**: Easy to add new frame limiting strategies

## Usage Examples

### Basic FPS Limiting
```cpp
gFPSController->SetTargetFPS(60.0);  // Limit to 60 FPS
```

### Check Current Performance
```cpp
double currentFps = gFPSController->GetCurrentFPS();
double frameTimeMs = gFPSController->GetFrameTime();
```

### Enable FPS Override from Config
```cpp
if (config.fpsCapping) {
    gFPSController->SetFPSOverride(true, config.targetFPS);
}
```

## Compilation Status
? **Successful** - All files compile without errors (C++17 compatible)

## Benefits
1. **Consistent Frame Rate**: Achieve target FPS across different hardware
2. **Reduced Power Consumption**: Intelligent sleep mechanism
3. **Network Stability**: Synchronized frame timing helps with server communication
4. **Flexible Control**: Easy to enable/disable or change FPS at runtime
5. **Production Ready**: Supports both legacy and optimized pipelines

## Notes
- The system integrates seamlessly with your existing `g_pTimer` and `CTimer` infrastructure
- Global variables allow runtime control from configuration files or UI
- The system automatically syncs with your CTime-based systems
- No changes required to existing rendering code

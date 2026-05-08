#pragma once

#include <chrono>
#include <thread>

/**
 * @file CFPSController.h
 * @brief FPS control and frame rate limiting system
 */

class CFPSController
{
public:
    static CFPSController* Instance();

    // Initialization
    bool Initialize();
    void Shutdown();

    // Frame timing
    void BeginFrame();
    void EndFrame();

    // FPS control
    void SetTargetFPS(double fps);
    double GetTargetFPS() const { return m_TargetFPS; }
    double GetCurrentFPS() const { return m_CurrentFPS; }
    double GetFrameTime() const { return m_FrameTimeMs; }

    // FPS override
    void SetFPSOverride(bool active, double overrideFps = 0.0);
    bool IsFPSOverrideActive() const { return m_FPSOverrideActive; }

    // Legacy pipeline support
    void SetUseOriginal150FPSPipeline(bool use) { m_UseOriginal150FPSPipeline = use; }
    bool GetUseOriginal150FPSPipeline() const { return m_UseOriginal150FPSPipeline; }

    virtual ~CFPSController();

private:
    CFPSController();
    static CFPSController* m_pInstance;

    bool m_bInitialized;
    double m_TargetFPS;
    double m_CurrentFPS;
    double m_FrameTimeMs;
    bool m_FPSOverrideActive;
    double m_FPSOverrideMs;
    bool m_UseOriginal150FPSPipeline;

    std::chrono::high_resolution_clock::time_point m_FrameStartTime;
    std::chrono::high_resolution_clock::time_point m_LastFrameTime;

    // Private methods
    void ApplyFrameLimiting();
};

// Global FPS control variables
extern int g_FPSOverrideActive;
extern double g_FPSOverrideMs;
extern bool g_UseOriginal150FPSPipeline;

#define gFPSController (CFPSController::Instance())

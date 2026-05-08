#include "stdafx.h"
#include "Time/CFPSController.h"
#include "Time/Timer.h"
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// Global FPS control variables
int g_FPSOverrideActive = 0;
double g_FPSOverrideMs = 0.0;
bool g_UseOriginal150FPSPipeline = false;

extern CTimer* g_pTimer;

CFPSController* CFPSController::m_pInstance = nullptr;

CFPSController::CFPSController()
    : m_bInitialized(false),
      m_TargetFPS(100.0),
      m_CurrentFPS(0.0),
      m_FrameTimeMs(0.0),
      m_FPSOverrideActive(false),
      m_FPSOverrideMs(0.0),
      m_UseOriginal150FPSPipeline(false)
{
}

CFPSController::~CFPSController()
{
    Shutdown();
}

CFPSController* CFPSController::Instance()
{
    if (!m_pInstance)
        m_pInstance = new CFPSController();
    return m_pInstance;
}

bool CFPSController::Initialize()
{
    if (m_bInitialized)
        return true;

    timeBeginPeriod(1); // Ensure 1ms sleep resolution for Windows scheduler

    m_bInitialized = true;
    m_FrameStartTime = std::chrono::high_resolution_clock::now();
    m_LastFrameTime = m_FrameStartTime;
    return true;
}

void CFPSController::Shutdown()
{
    if (m_bInitialized)
    {
        timeEndPeriod(1);
    }
    m_bInitialized = false;
}

void CFPSController::BeginFrame()
{
    m_FrameStartTime = std::chrono::high_resolution_clock::now();
}

void CFPSController::EndFrame()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto frameElapsed = std::chrono::duration<double>(now - m_FrameStartTime).count();
    m_FrameTimeMs = frameElapsed * 1000.0;

    // Calculate current FPS
    if (m_FrameTimeMs > 0.0)
    {
        m_CurrentFPS = 1000.0 / m_FrameTimeMs;
    }

    // Apply frame limiting
    ApplyFrameLimiting();

    m_LastFrameTime = now;
}

void CFPSController::SetTargetFPS(double fps)
{
    if (fps > 0.0)
    {
        m_TargetFPS = fps;
    }
}

void CFPSController::SetFPSOverride(bool active, double overrideFps)
{
    m_FPSOverrideActive = active;
    g_FPSOverrideActive = active ? 1 : 0;
    
    if (active && overrideFps > 0.0)
    {
        m_FPSOverrideMs = 1000.0 / overrideFps;
        g_FPSOverrideMs = m_FPSOverrideMs;
    }
}

void CFPSController::ApplyFrameLimiting()
{
    if (!m_bInitialized || !g_pTimer)
        return;

    // Disable arbitrary frame limits to ensure the absolute maximum FPS during gameplay (crucial for crowded zones).
    // Only apply limits if an override explicitly requests it.
    if (g_FPSOverrideActive == 0 || g_FPSOverrideMs <= 0.0)
    {
        return; // Uncapped frame rate -> Zero stutter.
    }

    double ms_per_frame = g_FPSOverrideMs;
    static double last_render_tick_count = 0.0;

    double now_ms = g_pTimer->GetAbsTime();
    
    if (last_render_tick_count == 0.0)
        last_render_tick_count = now_ms;

    double elapsed_ms = now_ms - last_render_tick_count;
    
    if (elapsed_ms < ms_per_frame)
    {
        double rest_ms = ms_per_frame - elapsed_ms;
        
        while (rest_ms > 0.0)
        {
            if (rest_ms > 2.0)
            {
                Sleep(1);
            }
            else
            {
                std::this_thread::yield();
            }
            now_ms = g_pTimer->GetAbsTime();
            elapsed_ms = now_ms - last_render_tick_count;
            rest_ms = ms_per_frame - elapsed_ms;
        }
    }

    last_render_tick_count = now_ms; // Update precisely after waiting
}

#include "stdafx.h"
#include "CPerformanceMonitor.h"
#include <iostream>

CPerformanceMonitor* CPerformanceMonitor::m_pInstance = nullptr;

CPerformanceMonitor::CPerformanceMonitor()
    : m_bInitialized(false),
      m_bProfilingEnabled(false),
      m_bAdaptiveQuality(true),
      m_bFPSDropped(false),
      m_MinFPSThreshold(100.0f),
      m_FPSDropAmount(0.0f),
      m_QualityScale(1.0f)
{
}

CPerformanceMonitor::~CPerformanceMonitor()
{
    Shutdown();
}

CPerformanceMonitor* CPerformanceMonitor::Instance()
{
    if (!m_pInstance)
        m_pInstance = new CPerformanceMonitor();
    return m_pInstance;
}

bool CPerformanceMonitor::Initialize()
{
    if (m_bInitialized)
        return true;

    m_CurrentMetrics = PerformanceMetrics{};
    m_bInitialized = true;

    return true;
}

void CPerformanceMonitor::Shutdown()
{
    m_bInitialized = false;
}

void CPerformanceMonitor::StartFrame()
{
    m_FrameStartTime = std::chrono::high_resolution_clock::now();
}

void CPerformanceMonitor::EndFrame()
{
    m_FrameEndTime = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        m_FrameEndTime - m_FrameStartTime).count();
    
    float frameTimeMs = duration / 1000.0f;
    float fps = 1000.0f / frameTimeMs;

    m_CurrentMetrics.FPS = fps;
    m_CurrentMetrics.FrameTime = frameTimeMs;

    // Armazenar histórico
    m_MetricsHistory.push_back(m_CurrentMetrics);
    if (m_MetricsHistory.size() > 300)
    {
        m_MetricsHistory.erase(m_MetricsHistory.begin());
    }

    // Detectar queda de FPS
    if (fps < m_MinFPSThreshold)
    {
        m_bFPSDropped = true;
        m_FPSDropAmount = m_MinFPSThreshold - fps;

        if (m_bAdaptiveQuality)
        {
            // Reduzir qualidade se FPS cair
            m_QualityScale = fps / m_MinFPSThreshold;
        }
    }
    else
    {
        m_bFPSDropped = false;
        m_FPSDropAmount = 0.0f;

        // Aumentar qualidade gradualmente
        if (m_QualityScale < 1.0f && m_bAdaptiveQuality)
        {
            float newScale = m_QualityScale + 0.01f;
            m_QualityScale = (newScale < 1.0f) ? newScale : 1.0f;
        }
    }
}

void CPerformanceMonitor::MarkCPUTime()
{
}

void CPerformanceMonitor::MarkGPUTime()
{
}

PerformanceMetrics CPerformanceMonitor::GetAverageMetrics(uint32_t samples) const
{
    if (m_MetricsHistory.empty())
        return PerformanceMetrics{};

    PerformanceMetrics avg{};
    uint32_t count = (samples < (uint32_t)m_MetricsHistory.size()) ? samples : (uint32_t)m_MetricsHistory.size();

    for (uint32_t i = 0; i < count; i++)
    {
        const auto& m = m_MetricsHistory[m_MetricsHistory.size() - 1 - i];
        avg.FPS += m.FPS;
        avg.FrameTime += m.FrameTime;
        avg.CpuTime += m.CpuTime;
        avg.GpuTime += m.GpuTime;
        avg.DrawCalls += m.DrawCalls;
        avg.Batches += m.Batches;
        avg.Triangles += m.Triangles;
    }

    avg.FPS /= count;
    avg.FrameTime /= count;
    avg.CpuTime /= count;
    avg.GpuTime /= count;
    avg.DrawCalls /= count;
    avg.Batches /= count;
    avg.Triangles /= count;

    return avg;
}

float CPerformanceMonitor::GetAverageFPS() const
{
    if (m_MetricsHistory.empty())
        return 0.0f;

    float total = 0.0f;
    for (const auto& m : m_MetricsHistory)
    {
        total += m.FPS;
    }

    return total / m_MetricsHistory.size();
}

void CPerformanceMonitor::PrintMetrics() const
{
    if (!m_bProfilingEnabled)
        return;

    auto avg = GetAverageMetrics(60);

    std::cout << "\n=== PERFORMANCE METRICS ===\n";
    std::cout << "FPS: " << avg.FPS << " (Target: 100+)\n";
    std::cout << "Frame Time: " << avg.FrameTime << " ms\n";
    std::cout << "CPU Time: " << avg.CpuTime << " ms\n";
    std::cout << "GPU Time: " << avg.GpuTime << " ms\n";
    std::cout << "Draw Calls: " << avg.DrawCalls << "\n";
    std::cout << "Batches: " << avg.Batches << "\n";
    std::cout << "Triangles: " << avg.Triangles << "\n";
    std::cout << "Quality Scale: " << m_QualityScale << "\n";

    if (m_bFPSDropped)
    {
        std::cout << "WARNING: FPS Dropped by " << m_FPSDropAmount << "\n";
    }

    std::cout << "===========================\n\n";
}

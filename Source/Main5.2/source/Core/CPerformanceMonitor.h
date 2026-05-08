#pragma once

#include <cstdint>
#include <vector>
#include <chrono>

/**
 * @file CPerformanceMonitor.h
 * @brief Monitor de performance e FPS para manter acima de 100 FPS
 */

struct PerformanceMetrics
{
    float FPS;
    float FrameTime;          // ms
    float CpuTime;            // ms
    float GpuTime;            // ms
    uint32_t DrawCalls;
    uint32_t Batches;
    uint32_t Triangles;
    float GpuMemoryUsage;     // MB
    float CpuMemoryUsage;     // MB
    float GpuUtilization;     // 0-100%
};

/**
 * @class CPerformanceMonitor
 * @brief Monitor de performance em tempo real
 */
class CPerformanceMonitor
{
public:
    static CPerformanceMonitor* Instance();

    // Inicialização
    bool Initialize();
    void Shutdown();

    // Medição
    void StartFrame();
    void EndFrame();
    void MarkCPUTime();
    void MarkGPUTime();

    // Estatísticas
    PerformanceMetrics GetCurrentMetrics() const { return m_CurrentMetrics; }
    PerformanceMetrics GetAverageMetrics(uint32_t samples = 60) const;
    float GetAverageFPS() const;

    // Alertas
    void SetFPSThreshold(float minFPS) { m_MinFPSThreshold = minFPS; }
    bool IsFPSDropped() const { return m_bFPSDropped; }
    float GetFPSDropAmount() const { return m_FPSDropAmount; }

    // Profiling
    void EnableProfiling(bool enable) { m_bProfilingEnabled = enable; }
    void PrintMetrics() const;

    // Otimizações automáticas
    void AdaptiveQuality(bool enable) { m_bAdaptiveQuality = enable; }
    float GetQualityScale() const { return m_QualityScale; }

    virtual ~CPerformanceMonitor();

private:
    CPerformanceMonitor();
    static CPerformanceMonitor* m_pInstance;

    bool m_bInitialized;
    bool m_bProfilingEnabled;
    bool m_bAdaptiveQuality;
    bool m_bFPSDropped;

    PerformanceMetrics m_CurrentMetrics;
    std::vector<PerformanceMetrics> m_MetricsHistory;

    float m_MinFPSThreshold;
    float m_FPSDropAmount;
    float m_QualityScale;

    std::chrono::high_resolution_clock::time_point m_FrameStartTime;
    std::chrono::high_resolution_clock::time_point m_FrameEndTime;
};

#define gPerformanceMonitor (CPerformanceMonitor::Instance())

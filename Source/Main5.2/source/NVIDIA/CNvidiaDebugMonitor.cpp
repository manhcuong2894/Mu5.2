#include "stdafx.h"
#include "CNvidiaDebugMonitor.h"
#include "CNvidiaSystem.h"
#include <GL/glew.h>
#include <iostream>
#include <ctime>
#include <chrono>

CNvidiaDebugMonitor* CNvidiaDebugMonitor::m_pInstance = nullptr;

CNvidiaDebugMonitor::CNvidiaDebugMonitor()
    : m_bInitialized(false)
    , m_bDebugEnabled(true)
    , m_bShowHUD(true)
    , m_UpdateInterval(1.0f)
    , m_TimeSinceLastUpdate(0.0f)
    , m_FrameTimeIndex(0)
{
    memset(&m_Stats, 0, sizeof(m_Stats));
    memset(&m_PreviousStats, 0, sizeof(m_PreviousStats));
    memset(m_FrameTimeBuffer, 0, sizeof(m_FrameTimeBuffer));
}

CNvidiaDebugMonitor::~CNvidiaDebugMonitor()
{
    Shutdown();
}

CNvidiaDebugMonitor* CNvidiaDebugMonitor::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CNvidiaDebugMonitor();
    }
    return m_pInstance;
}

bool CNvidiaDebugMonitor::Initialize()
{
    if (m_bInitialized)
        return true;

    m_bInitialized = true;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  NVIDIA Debug Monitor Inicializado" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Verificação inicial
    if (!gNvidiaGPU->IsNvidiaSupported())
    {
        std::cout << "[DEBUG] ??  NVIDIA GPU não detectada!" << std::endl;
        m_Stats.isNvidiaDetected = false;
    }
    else
    {
        std::cout << "[DEBUG] ? NVIDIA GPU DETECTADA!" << std::endl;
        m_Stats.isNvidiaDetected = true;
        
        // Obter informações da GPU
        if (gNvidiaGPU->GetGPUCount() > 0)
        {
            const auto& gpuInfo = gNvidiaGPU->GetGPUInfo(0);
            m_Stats.gpuName = gpuInfo.DeviceName;
            m_Stats.gpuMemoryTotalMB = gpuInfo.TotalMemory / 1024 / 1024;
            
            std::cout << "[DEBUG] GPU: " << gpuInfo.DeviceName << std::endl;
            std::cout << "[DEBUG] Memória Total: " << m_Stats.gpuMemoryTotalMB << " MB" << std::endl;
        }
    }

    return true;
}

void CNvidiaDebugMonitor::Shutdown()
{
    if (!m_bInitialized)
        return;

    m_bInitialized = false;
    std::cout << "[DEBUG] Monitor finalizado!" << std::endl;
}

void CNvidiaDebugMonitor::UpdateStats()
{
    if (!m_bInitialized)
        return;

    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    // Calcular FPS
    m_Stats.frameTimeMS = deltaTime * 1000.0f;
    m_FrameTimeBuffer[m_FrameTimeIndex] = m_Stats.frameTimeMS;
    m_FrameTimeIndex = (m_FrameTimeIndex + 1) % 60;

    CalculateFPS();

    // Update stats periódicos
    m_TimeSinceLastUpdate += deltaTime;
    if (m_TimeSinceLastUpdate >= m_UpdateInterval)
    {
        UpdateGPUStats();
        UpdateRenderStats();
        UpdateMemoryStats();
        m_TimeSinceLastUpdate = 0.0f;

        // Debug output
        if (m_bDebugEnabled)
        {
            PrintStatsToConsole();
        }
    }
}

void CNvidiaDebugMonitor::UpdateGPUStats()
{
    if (!gNvidiaGPU->IsNvidiaSupported())
    {
        m_Stats.isNvidiaDetected = false;
        m_Stats.gpuUtilization = 0.0f;
        return;
    }

    m_Stats.isNvidiaDetected = true;

    // Obter estatísticas da GPU
    const auto& gpuStats = gNvidiaGPU->GetRenderStats();
    m_Stats.gpuUtilization = gpuStats.GPUUsage;
    m_Stats.gpuMemoryUsage = gpuStats.MemoryUsage;

    // Informações de memória
    size_t usedMemory = gNvidiaGPU->GetUsedMemory();
    size_t availableMemory = gNvidiaGPU->GetAvailableMemory();
    m_Stats.gpuMemoryUsedMB = usedMemory / 1024 / 1024;
    m_Stats.gpuMemoryTotalMB = (usedMemory + availableMemory) / 1024 / 1024;
}

void CNvidiaDebugMonitor::UpdateRenderStats()
{
    // Obter informações de renderização
    m_Stats.isGPURenderingActive = gNvidiaGPU->IsGPURenderingEnabled();
    
    // Shader ativo
    GLint shaderProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProg);
    m_Stats.activeShaderProgram = (GLuint)shaderProg;
    m_Stats.shaderActive = (m_Stats.activeShaderProgram != 0);
}

void CNvidiaDebugMonitor::UpdateMemoryStats()
{
    // Memoria de buffers
    GLint totalBufferMemory = 0;
    glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &totalBufferMemory);
    
    m_Stats.bufferMemory = (totalBufferMemory > 0) ? totalBufferMemory / 1024 : 0;
}

void CNvidiaDebugMonitor::CalculateFPS()
{
    float totalTime = 0.0f;
    for (int i = 0; i < 60; ++i)
    {
        totalTime += m_FrameTimeBuffer[i];
    }
    
    float avgFrameTime = totalTime / 60.0f;
    if (avgFrameTime > 0.0f)
    {
        m_Stats.fps = 1000.0f / avgFrameTime;
    }
}

void CNvidiaDebugMonitor::PrintStatsToConsole()
{
    std::cout << "\n??????????????????????????????????????????" << std::endl;
    std::cout << "?     NVIDIA DEBUG MONITOR - STATUS      ?" << std::endl;
    std::cout << "??????????????????????????????????????????" << std::endl;

    // GPU Status
    std::cout << "\n?? GPU STATUS:" << std::endl;
    if (m_Stats.isNvidiaDetected)
    {
        std::cout << "  ? GPU NVIDIA: DETECTADA" << std::endl;
        std::cout << "  ?? GPU Name: " << m_Stats.gpuName << std::endl;
    }
    else
    {
        std::cout << "  ? GPU NVIDIA: NÃO DETECTADA" << std::endl;
    }

    // GPU Utilization
    std::cout << "\n? UTILIZAÇÃO:" << std::endl;
    std::cout << "  GPU Usage:    " << m_Stats.gpuUtilization << "%" << std::endl;
    std::cout << "  Memory:       " << m_Stats.gpuMemoryUsage << "% (" 
              << m_Stats.gpuMemoryUsedMB << "/" << m_Stats.gpuMemoryTotalMB << " MB)" << std::endl;

    // Rendering Status
    std::cout << "\n?? RENDERIZAÇÃO:" << std::endl;
    std::cout << "  GPU Rendering: " << (m_Stats.isGPURenderingActive ? "? ATIVO" : "? INATIVO") << std::endl;
    std::cout << "  Shader Ativo:   " << (m_Stats.shaderActive ? "? SIM" : "? NÃO") << std::endl;

    // Performance
    std::cout << "\n?? PERFORMANCE:" << std::endl;
    std::cout << "  FPS:          " << m_Stats.fps << std::endl;
    std::cout << "  Frame Time:   " << m_Stats.frameTimeMS << " ms" << std::endl;
    std::cout << "  Draw Calls:   " << m_Stats.drawCalls << std::endl;

    std::cout << "\n????????????????????????????????????????\n" << std::endl;
}

void CNvidiaDebugMonitor::LogStatsToFile(const char* filename)
{
    FILE* file = nullptr;
    fopen_s(&file, filename, "a");
    
    if (!file)
        return;

    time_t now = time(0);
    tm timeinfo = {};
    localtime_s(&timeinfo, &now);
    
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

    fprintf(file, "[%s] GPU: %.1f%% | Memory: %.1f%% | FPS: %.1f | FrameTime: %.2f ms\n",
        timeStr,
        m_Stats.gpuUtilization,
        m_Stats.gpuMemoryUsage,
        m_Stats.fps,
        m_Stats.frameTimeMS
    );

    fclose(file);
}

void CNvidiaDebugMonitor::RenderDebugOverlay()
{
    if (!m_bShowHUD)
        return;

    // Este método seria implementado com seu sistema de renderização de HUD
    // Exemplo de como renderizar o texto na tela
}

void CNvidiaDebugMonitor::RenderPerformanceGraph()
{
    // Renderizar gráfico de performance
    // Implementar usando seu sistema de renderização
}

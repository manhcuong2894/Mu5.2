#include "stdafx.h"
#include "CNvidiaSystem.h"
#include <iostream>

CNvidiaSystem* CNvidiaSystem::m_pInstance = nullptr;

CNvidiaSystem::CNvidiaSystem()
    : m_bInitialized(false)
{
}

CNvidiaSystem::~CNvidiaSystem()
{
    Shutdown();
}

CNvidiaSystem* CNvidiaSystem::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CNvidiaSystem();
    }
    return m_pInstance;
}

bool CNvidiaSystem::Initialize()
{
    if (m_bInitialized)
        return true;

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Inicializando Sistema NVIDIA" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Inicializar GPU Manager
    if (!gNvidiaGPU->Initialize())
    {
        std::cerr << "[NVIDIA System] Falha ao inicializar GPU Manager!" << std::endl;
        return false;
    }

    // Inicializar Memory Manager
    size_t memorySize = 512 * 1024 * 1024; // 512 MB
    if (!gNvidiaMemory->Initialize(memorySize))
    {
        std::cerr << "[NVIDIA System] Falha ao inicializar Memory Manager!" << std::endl;
        return false;
    }

    // Inicializar Render System
    CNvidiaRenderSystem::RenderConfig renderConfig = {};
    renderConfig.Type = CNvidiaRenderSystem::RENDER_TYPE_DEFERRED;
    renderConfig.EnableSSAO = true;
    renderConfig.EnableSSR = false;
    renderConfig.EnableMotionBlur = false;
    renderConfig.EnableTAA = true;
    renderConfig.EnableAsyncCompute = true;
    renderConfig.MaxLights = 32;
    renderConfig.ShadowResolution = 2048;

    if (!gNvidiaRender->Initialize(renderConfig))
    {
        std::cerr << "[NVIDIA System] Falha ao inicializar Render System!" << std::endl;
        return false;
    }

    // Habilitar renderização via GPU
    gNvidiaGPU->EnableGPURendering(true);

    m_bInitialized = true;

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Sistema NVIDIA Inicializado com Sucesso!" << std::endl;
    std::cout << "========================================\n" << std::endl;

    PrintSystemInfo();

    return true;
}

void CNvidiaSystem::Shutdown()
{
    if (!m_bInitialized)
        return;

    std::cout << "[NVIDIA System] Finalizando sistema..." << std::endl;

    gNvidiaRender->Shutdown();
    gNvidiaMemory->Shutdown();
    gNvidiaGPU->Shutdown();

    m_bInitialized = false;

    std::cout << "[NVIDIA System] Sistema finalizado!" << std::endl;
}

void CNvidiaSystem::BeginFrame()
{
    if (!m_bInitialized)
        return;

    gNvidiaGPU->UpdateRenderStats();
}

void CNvidiaSystem::EndFrame()
{
    if (!m_bInitialized)
        return;

    gNvidiaRender->FlushGPUCommands();
    
    // Atualizar debug monitor
    gNvidiaDebug->UpdateStats();
}

void CNvidiaSystem::EnableDebugOutput(bool enable)
{
    gNvidiaDebug->EnableDebugOutput(enable);
    gNvidiaDebug->EnableHUDOverlay(enable);
}

void CNvidiaSystem::PrintSystemInfo()
{
    std::cout << "\n--- Informações do Sistema NVIDIA ---" << std::endl;
    std::cout << "GPUs Detectadas: " << gNvidiaGPU->GetGPUCount() << std::endl;

    for (int i = 0; i < gNvidiaGPU->GetGPUCount(); ++i)
    {
        const auto& gpuInfo = gNvidiaGPU->GetGPUInfo(i);
        std::cout << "\nGPU " << i << ":" << std::endl;
        std::cout << "  Nome: " << gpuInfo.DeviceName << std::endl;
        std::cout << "  Memória Total: " << (gpuInfo.TotalMemory / 1024 / 1024) << " MB" << std::endl;
        std::cout << "  Compute Capability: " << (gpuInfo.ComputeCapability / 10) << "." << (gpuInfo.ComputeCapability % 10) << std::endl;
        std::cout << "  Clock Rate: " << gpuInfo.ClockRate << " MHz" << std::endl;
        std::cout << "  Max Threads per Block: " << gpuInfo.MaxThreadsPerBlock << std::endl;
    }

    std::cout << "\nMemória Alocada: " << (gNvidiaMemory->GetUsedMemory() / 1024 / 1024) << " MB / " 
              << (gNvidiaMemory->GetTotalMemory() / 1024 / 1024) << " MB" << std::endl;
    std::cout << "Blocos de Memória: " << gNvidiaMemory->GetBlockCount() << std::endl;
    std::cout << "Renderização via GPU: " << (gNvidiaGPU->IsGPURenderingEnabled() ? "ATIVADA" : "DESATIVADA") << std::endl;
    std::cout << "\n-----------------------------------\n" << std::endl;
}

void CNvidiaSystem::PrintPerformanceStats()
{
    const auto& stats = gNvidiaGPU->GetRenderStats();

    std::cout << "\n--- Performance Stats ---" << std::endl;
    std::cout << "GPU Usage: " << stats.GPUUsage << "%" << std::endl;
    std::cout << "Memory Usage: " << stats.MemoryUsage << "%" << std::endl;
    std::cout << "Temperature: " << stats.Temperature << "°C" << std::endl;
    std::cout << "FPS: " << stats.FPS << std::endl;
    std::cout << "Frames Rendered: " << stats.FramesRendered << std::endl;
    std::cout << "------------------------\n" << std::endl;
}

#pragma once

#include "CNvidiaGPUManager.h"
#include "CNvidiaRenderSystem.h"
#include "CNvidiaMemoryManager.h"
#include "CNvidiaDebugMonitor.h"

// Forward declare
class CGPURenderingManager;

/**
 * @brief Sistema integrado de renderização NVIDIA
 * @details Gerencia GPU, renderização e memória em um único ponto
 */
class CNvidiaSystem
{
    // Permitir que CGPURenderingManager acesse o construtor privado
    friend class CGPURenderingManager;

public:
    static CNvidiaSystem* Instance();

    // Inicialização completa do sistema
    bool Initialize();
    void Shutdown();

    // Acessadores
    CNvidiaGPUManager* GetGPUManager() const { return gNvidiaGPU; }
    CNvidiaRenderSystem* GetRenderSystem() const { return gNvidiaRender; }
    CNvidiaMemoryManager* GetMemoryManager() const { return gNvidiaMemory; }
    CNvidiaDebugMonitor* GetDebugMonitor() const { return gNvidiaDebug; }

    // Status
    bool IsInitialized() const { return m_bInitialized; }
    bool IsGPURenderingEnabled() const { return gNvidiaGPU->IsGPURenderingEnabled(); }

    // Gerenciamento de frame
    void BeginFrame();
    void EndFrame();

    // Debug e profiling
    void PrintSystemInfo();
    void PrintPerformanceStats();
    void EnableDebugOutput(bool enable);

    virtual ~CNvidiaSystem();

private:
    CNvidiaSystem();
    
    static CNvidiaSystem* m_pInstance;
    bool m_bInitialized;
};

#define gNvidiaSystem (CNvidiaSystem::Instance())

#pragma once

#include <Windows.h>
#include <GL/glew.h>
#include <vector>
#include <memory>

/**
 * @brief Gerenciador de GPU NVIDIA
 * @details Classe responsável por gerenciar a renderização via GPU NVIDIA
 * Suporta detecção automática de GPUs, otimizações de memória e renderização paralela
 */
class CNvidiaGPUManager
{
public:
    // Estrutura para informações da GPU
    struct GPUInfo
    {
        int DeviceID;
        char DeviceName[256];
        size_t TotalMemory;
        size_t AvailableMemory;
        bool IsSupported;
        int ComputeCapability;
        float ClockRate;
        int MaxThreadsPerBlock;
    };

    // Estrutura para estatísticas de renderização
    struct RenderStats
    {
        float GPUUsage;
        float MemoryUsage;
        float Temperature;
        float FPS;
        unsigned long long FramesRendered;
    };

    static CNvidiaGPUManager* Instance();

    // Inicialização e finalização
    bool Initialize();
    void Shutdown();

    // Gerenciamento de GPU
    bool DetectGPUs();
    int GetGPUCount() const;
    const GPUInfo& GetGPUInfo(int deviceID) const;
    bool SelectGPU(int deviceID);

    // Gerenciamento de memória
    bool AllocateGPUMemory(size_t size, GLuint& buffer);
    bool FreeGPUMemory(GLuint buffer);
    size_t GetAvailableMemory() const;
    size_t GetUsedMemory() const;

    // Renderização otimizada
    bool EnableGPURendering(bool enable);
    bool IsGPURenderingEnabled() const;
    
    // Processamento de geometria
    bool ProcessMeshOnGPU(const void* meshData, size_t meshSize);
    
    // Estatísticas e monitoramento
    const RenderStats& GetRenderStats() const;
    void UpdateRenderStats();
    
    // Utilidades
    bool IsNvidiaSupported() const;
    bool IsCUDAAvailable() const;
    
    virtual ~CNvidiaGPUManager();

private:
    CNvidiaGPUManager();
    
    static CNvidiaGPUManager* m_pInstance;
    
    std::vector<GPUInfo> m_GPUs;
    int m_CurrentDeviceID;
    bool m_bInitialized;
    bool m_bGPURenderingEnabled;
    RenderStats m_RenderStats;
    GLuint m_GPUBuffer;
    
    // Métodos privados
    bool QueryGPUCapabilities();
    bool SetupGPUContext();
    void UpdateMemoryStats();
};

#define gNvidiaGPU (CNvidiaGPUManager::Instance())

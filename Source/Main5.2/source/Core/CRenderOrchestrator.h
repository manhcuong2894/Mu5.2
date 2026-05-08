#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

/**
 * @file CRenderOrchestrator.h
 * @brief Sistema central de orquestração para renderização harmonizada
 * @details Coordena Shaders, Scaleform, CColorSystem e GPUs (NVIDIA/AMD/Intel)
 * 
 * Características:
 * - Batching automático de renderizações
 * - Sincronização entre subsistemas
 * - Load balancing de GPU
 * - FPS locking (100+ FPS)
 * - Detecção automática de GPU
 * - Fallback para CPU se necessário
 */

// Prioridades de renderização
enum class RenderPriority : uint8_t
{
    TERRAIN = 0,           // Renderizado primeiro (NVIDIA)
    OBJECTS = 1,           // Objetos do mundo
    CHARACTERS = 2,        // Personagens com animação
    PARTICLES = 3,         // Efeitos
    UI_GAME = 4,           // UI in-game (Scaleform)
    UI_OVERLAY = 5         // UI overlay (Scaleform)
};

// Estados de GPU
enum class GPUVendor
{
    NVIDIA = 0,
    AMD = 1,
    INTEL = 2,
    FALLBACK = 3
};

// Informações de batch
struct RenderBatch
{
    RenderPriority Priority;
    GLuint VAO;
    GLuint ShaderProgram;
    GLuint Texture;
    uint32_t IndexCount;
    uint32_t InstanceCount;
    glm::mat4 ModelMatrix;
    bool UseInstancing;
    bool UseTessellation;
    GPUVendor PreferredGPU;
};

// Estatísticas de performance
struct FrameStats
{
    float FPS;
    float FrameTime;
    float GPUTime;
    float CPUTime;
    uint32_t DrawCalls;
    uint32_t TrianglesRendered;
    uint32_t BatchesSubmitted;
    float GPUUtilization;
};

/**
 * @class CRenderOrchestrator
 * @brief Orquestrador central de renderização
 */
class CRenderOrchestrator
{
public:
    static CRenderOrchestrator* Instance();

    // Inicialização
    bool Initialize();
    void Shutdown();
    bool IsInitialized() const { return m_bInitialized; }

    // Gerenciamento de batches
    void SubmitBatch(const RenderBatch& batch);
    void ClearBatches();
    void OptimizeBatches();

    // Renderização
    void BeginFrame();
    void RenderFrame();
    void EndFrame();
    void Present();

    // Sincronização entre subsistemas
    void SyncShaderState();
    void SyncScaleformState();
    void SyncColorSystem();
    void SyncGPUState();

    // Gerenciamento de GPU
    GPUVendor GetDetectedGPU() const { return m_DetectedGPU; }
    bool SetPreferredGPU(GPUVendor vendor);
    bool UseGPUAcceleration(bool enable);
    bool IsGPUAccelerationActive() const { return m_bGPUAccelerationActive; }

    // Performance
    FrameStats GetFrameStats() const { return m_FrameStats; }
    float GetAverageFPS(uint32_t samples = 60) const;
    void LockFPS(uint32_t targetFPS);
    void UnlockFPS();
    bool IsFPSLocked() const { return m_bFPSLocked; }

    // Configuração
    struct Config
    {
        bool EnableBatching = true;
        bool EnableInstancing = true;
        bool EnableTessellation = false;  // AMD/NVIDIA
        bool EnableComputeShaders = false;
        uint32_t MaxBatchSize = 10000;
        uint32_t MaxDrawCalls = 5000;
        bool EnableVSync = false;
        bool UseAsyncCompute = true;      // NVIDIA
        uint32_t TargetFPS = 100;
        bool VerboseLogging = false;
    };

    void SetConfig(const Config& config);
    const Config& GetConfig() const { return m_Config; }

    // Debug
    void PrintRenderStats() const;
    void PrintGPUInfo() const;
    void ToggleProfiling(bool enable);
    bool IsProfilingEnabled() const { return m_bProfilingEnabled; }

    virtual ~CRenderOrchestrator();

private:
    CRenderOrchestrator();

    static CRenderOrchestrator* m_pInstance;

    bool m_bInitialized;
    bool m_bGPUAccelerationActive;
    bool m_bFPSLocked;
    bool m_bProfilingEnabled;

    Config m_Config;
    GPUVendor m_DetectedGPU;
    GPUVendor m_PreferredGPU;

    // Batches
    std::vector<RenderBatch> m_BatchQueue;
    std::map<RenderPriority, std::vector<RenderBatch>> m_SortedBatches;

    // Estatísticas
    FrameStats m_FrameStats;
    std::vector<float> m_FPSHistory;
    uint32_t m_FrameCount;
    double m_LastFrameTime;
    double m_FrameStartTime;
    uint32_t m_TargetFrameTime;  // em microsegundos

    // GPU queries
    GLuint m_GPUTimeQuery;
    GLuint m_PrimitiveQuery;

    // Métodos privados
    void DetectGPU();
    void SelectRenderPath();
    void SubmitBatchesToGPU();
    void OptimizeForGPU(GPUVendor vendor);
    void MeasureFrameTime();
    void EnforceFrameRate();
    void UpdateFrameStats();
};

#define gRenderOrchestrator (CRenderOrchestrator::Instance())

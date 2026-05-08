#pragma once

#include "CGPUDetector.h"
#include "../CNvidiaSystem.h"
#include <memory>

/**
 * @file CGPURenderingManager.h
 * @brief Gerenciador automático de renderização por GPU
 * @details Detecta GPU e ativa o sistema de renderização apropriado
 * 
 * Suporta:
 * - NVIDIA: Sistema NVIDIA otimizado
 * - AMD: Renderização OpenGL compatible com fallback
 * - Intel: Renderização forward com otimizações Intel
 * - Outros: Renderização básica OpenGL
 */

class CGPURenderingManager
{
public:
    // Estados de renderização
    enum class RenderingMode
    {
        NVIDIA_OPTIMIZED = 0,       // NVIDIA com otimizações especiais
        AMD_COMPATIBLE = 1,         // AMD com renderização deferred
        INTEL_OPTIMIZED = 2,        // Intel com renderização forward
        FALLBACK_FORWARD = 3,       // Forward rendering básico
        FALLBACK_CPU = 4            // CPU rendering (último recurso)
    };

    // Estrutura de configuração automática
    struct AutoConfig
    {
        bool AutoDetect;            // Detectar automaticamente
        bool EnableOptimizations;   // Ativar otimizações específicas
        bool AllowFallback;         // Permitir fallback
        bool VerboseOutput;         // Output detalhado
        
        // Overrides (se não quer auto-detect)
        CGPUDetector::GPUType ForceGPUType;
        RenderingMode ForcedMode;
    };

    static CGPURenderingManager* Instance();

    // Inicialização
    bool Initialize(const AutoConfig& config = GetDefaultConfig());
    void Shutdown();

    // Gerenciamento
    RenderingMode GetRenderingMode() const { return m_RenderingMode; }
    CGPUDetector::GPUType GetDetectedGPU() const { return m_DetectedGPU; }
    std::string GetRenderingModeString() const;
    std::string GetGPUInfoString() const;

    // Renderização específica por GPU
    bool IsNVIDIAOptimized() const { return m_RenderingMode == RenderingMode::NVIDIA_OPTIMIZED; }
    bool IsAMDOptimized() const { return m_RenderingMode == RenderingMode::AMD_COMPATIBLE; }
    bool IsIntelOptimized() const { return m_RenderingMode == RenderingMode::INTEL_OPTIMIZED; }
    bool IsFallback() const { return m_RenderingMode == RenderingMode::FALLBACK_FORWARD || 
                                    m_RenderingMode == RenderingMode::FALLBACK_CPU; }

    // Features disponíveis
    bool SupportsDeferred() const;
    bool SupportsCompute() const;
    bool SupportsTessellation() const;
    bool SupportsInstancing() const;
    bool SupportsAsyncCompute() const;

    // Configuração automática
    void PrintDetectionResults();
    void LogToFile(const char* filename);

    // Acesso aos sistemas
    static std::unique_ptr<CNvidiaSystem> CreateNvidiaSystem();
    CNvidiaSystem* GetNvidiaSystem() const { return m_pNvidiaSystem.get(); }

    virtual ~CGPURenderingManager();

private:
    CGPURenderingManager();

    static CGPURenderingManager* m_pInstance;
    static AutoConfig GetDefaultConfig();

    // Detecção e inicialização
    bool DetectAndInitialize(const AutoConfig& config);
    RenderingMode ChooseRenderingMode(CGPUDetector::GPUType gpuType);
    bool InitializeRenderingSystem();

    // Membros
    RenderingMode m_RenderingMode;
    CGPUDetector::GPUType m_DetectedGPU;
    AutoConfig m_Config;
    
    std::unique_ptr<CNvidiaSystem> m_pNvidiaSystem;
    
    bool m_bInitialized;
};

#define gGPURenderingManager (CGPURenderingManager::Instance())

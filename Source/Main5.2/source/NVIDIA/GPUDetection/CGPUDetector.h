#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>

/**
 * @file CGPUDetector.h
 * @brief Detector automático de GPU (NVIDIA, AMD, Intel)
 * @details Identifica a GPU do sistema e retorna informações detalhadas
 */

class CGPUDetector
{
public:
    // Tipos de GPU suportadas
    enum class GPUType
    {
        NVIDIA = 0,
        AMD = 1,
        INTEL = 2,
        OTHER = 3,
        UNKNOWN = 4
    };

    // Informações da GPU
    struct GPUInfo
    {
        GPUType Type;
        std::string Name;
        std::string Vendor;
        std::string Renderer;
        std::string Version;
        std::string Driver;
        
        // Capacidades
        int MaxTextureSize;
        int MaxTextureUnits;
        int MaxComputeWorkGroupSize[3];
        
        // Memory
        size_t TotalMemory;
        bool SupportsUnicode;
        
        // Features
        bool SupportsCompute;
        bool SupportsDeferred;
        bool SupportsTessellation;
        bool SupportsGeometryShader;
        bool SupportsInstancing;
    };

    static CGPUDetector* Instance();

    // Detecção
    bool DetectGPU();
    GPUType GetGPUType() const { return m_DetectedGPU.Type; }
    const GPUInfo& GetGPUInfo() const { return m_DetectedGPU; }

    // Validação
    bool IsNVIDIASupported() const { return m_DetectedGPU.Type == GPUType::NVIDIA; }
    bool IsAMDSupported() const { return m_DetectedGPU.Type == GPUType::AMD; }
    bool IsIntelSupported() const { return m_DetectedGPU.Type == GPUType::INTEL; }
    
    // Retorno do tipo como string
    std::string GetGPUTypeString() const;
    std::string GetGPUTypeAsText() const;

    // Print info
    void PrintGPUInfo();
    void LogGPUInfo(const char* filename);

    // Fallback chain
    bool SupportsOptimalRendering() const;
    bool CanFallbackToDeferred() const;
    bool CanFallbackToForward() const;

    virtual ~CGPUDetector();

private:
    CGPUDetector();

    static CGPUDetector* m_pInstance;

    GPUInfo m_DetectedGPU;
    bool m_bDetected;

    // Métodos privados de detecção
    void QueryGPUInfo();
    void IdentifyVendor();
    void QueryCapabilities();
    void DetectFeatures();
    
    // Helpers
    bool ContainsString(const std::string& text, const std::string& search) const;
    size_t ExtractMemoryFromString(const std::string& text) const;
};

#define gGPUDetector (CGPUDetector::Instance())

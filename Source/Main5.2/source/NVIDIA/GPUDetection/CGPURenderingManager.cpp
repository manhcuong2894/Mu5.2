#include "stdafx.h"
#include "CGPURenderingManager.h"
#include <iostream>

CGPURenderingManager* CGPURenderingManager::m_pInstance = nullptr;

CGPURenderingManager::CGPURenderingManager()
    : m_RenderingMode(RenderingMode::FALLBACK_CPU)
    , m_DetectedGPU(CGPUDetector::GPUType::UNKNOWN)
    , m_bInitialized(false)
{
    m_Config = GetDefaultConfig();
}

CGPURenderingManager::~CGPURenderingManager()
{
    Shutdown();
}

CGPURenderingManager* CGPURenderingManager::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CGPURenderingManager();
    }
    return m_pInstance;
}

CGPURenderingManager::AutoConfig CGPURenderingManager::GetDefaultConfig()
{
    AutoConfig config = {};
    config.AutoDetect = true;
    config.EnableOptimizations = true;
    config.AllowFallback = true;
    config.VerboseOutput = true;
    config.ForceGPUType = CGPUDetector::GPUType::UNKNOWN;
    config.ForcedMode = RenderingMode::FALLBACK_CPU;
    return config;
}

bool CGPURenderingManager::Initialize(const AutoConfig& config)
{
    if (m_bInitialized)
        return true;

    m_Config = config;

    std::cout << "\n??????????????????????????????????????????" << std::endl;
    std::cout << "?   GPU Rendering Manager Init          ?" << std::endl;
    std::cout << "??????????????????????????????????????????\n" << std::endl;

    // Detectar e inicializar
    if (!DetectAndInitialize(config))
    {
        std::cerr << "[GPU Manager] Falha na inicialização!" << std::endl;
        return false;
    }

    m_bInitialized = true;

    PrintDetectionResults();

    return true;
}

void CGPURenderingManager::Shutdown()
{
    if (!m_bInitialized)
        return;

    if (m_pNvidiaSystem)
    {
        m_pNvidiaSystem->Shutdown();
        m_pNvidiaSystem.reset();
    }

    m_bInitialized = false;
    std::cout << "[GPU Manager] Sistema finalizado!" << std::endl;
}

bool CGPURenderingManager::DetectAndInitialize(const AutoConfig& config)
{
    // Detectar GPU
    if (!gGPUDetector->DetectGPU())
    {
        std::cerr << "[GPU Manager] Falha na detecção de GPU!" << std::endl;
        return false;
    }

    m_DetectedGPU = gGPUDetector->GetGPUType();

    // Escolher modo de renderização
    m_RenderingMode = ChooseRenderingMode(m_DetectedGPU);

    // Inicializar sistema de renderização
    if (!InitializeRenderingSystem())
    {
        std::cerr << "[GPU Manager] Falha ao inicializar sistema de renderização!" << std::endl;
        return false;
    }

    return true;
}

CGPURenderingManager::RenderingMode CGPURenderingManager::ChooseRenderingMode(
    CGPUDetector::GPUType gpuType)
{
    const auto& gpuInfo = gGPUDetector->GetGPUInfo();

    std::cout << "[GPU Manager] Escolhendo modo de renderização..." << std::endl;

    // NVIDIA - Sistema otimizado completo
    if (gpuType == CGPUDetector::GPUType::NVIDIA)
    {
        if (gpuInfo.SupportsCompute && gpuInfo.SupportsDeferred)
        {
            std::cout << "[GPU Manager] ? Modo NVIDIA OPTIMIZED ativado!" << std::endl;
            return RenderingMode::NVIDIA_OPTIMIZED;
        }
    }

    // AMD - Renderização deferred compatible
    if (gpuType == CGPUDetector::GPUType::AMD)
    {
        if (gpuInfo.SupportsDeferred && gpuInfo.MaxTextureUnits >= 8)
        {
            std::cout << "[GPU Manager] ? Modo AMD COMPATIBLE ativado!" << std::endl;
            return RenderingMode::AMD_COMPATIBLE;
        }
    }

    // Intel - Renderização forward otimizada
    if (gpuType == CGPUDetector::GPUType::INTEL)
    {
        if (gpuInfo.SupportsInstancing)
        {
            std::cout << "[GPU Manager] ? Modo INTEL OPTIMIZED ativado!" << std::endl;
            return RenderingMode::INTEL_OPTIMIZED;
        }
    }

    // Fallback - Forward rendering básico
    if (m_Config.AllowFallback)
    {
        std::cout << "[GPU Manager] ??  Usando FALLBACK FORWARD RENDERING!" << std::endl;
        return RenderingMode::FALLBACK_FORWARD;
    }

    // Último recurso
    std::cout << "[GPU Manager] ??  Usando FALLBACK CPU RENDERING!" << std::endl;
    return RenderingMode::FALLBACK_CPU;
}

bool CGPURenderingManager::InitializeRenderingSystem()
{
    switch (m_RenderingMode)
    {
    case RenderingMode::NVIDIA_OPTIMIZED:
    case RenderingMode::AMD_COMPATIBLE:
    case RenderingMode::INTEL_OPTIMIZED:
    {
        // Inicializar sistema NVIDIA (funciona para todos com GLEW)
        m_pNvidiaSystem = CGPURenderingManager::CreateNvidiaSystem();
        if (!m_pNvidiaSystem || !m_pNvidiaSystem->Initialize())
        {
            std::cerr << "[GPU Manager] Falha ao inicializar NVIDIA System!" << std::endl;
            m_pNvidiaSystem.reset();
            return false;
        }
        return true;
    }

    case RenderingMode::FALLBACK_FORWARD:
    {
        std::cout << "[GPU Manager] Forward rendering será usado" << std::endl;
        return true;
    }

    case RenderingMode::FALLBACK_CPU:
    {
        std::cout << "[GPU Manager] CPU rendering será usado" << std::endl;
        return true;
    }

    default:
        return false;
    }
}

std::unique_ptr<CNvidiaSystem> CGPURenderingManager::CreateNvidiaSystem()
{
    // Usar new diretamente já que o construtor é privado
    return std::unique_ptr<CNvidiaSystem>(new CNvidiaSystem());
}

std::string CGPURenderingManager::GetRenderingModeString() const
{
    switch (m_RenderingMode)
    {
    case RenderingMode::NVIDIA_OPTIMIZED:
        return "NVIDIA Optimized";
    case RenderingMode::AMD_COMPATIBLE:
        return "AMD Compatible";
    case RenderingMode::INTEL_OPTIMIZED:
        return "Intel Optimized";
    case RenderingMode::FALLBACK_FORWARD:
        return "Fallback Forward";
    case RenderingMode::FALLBACK_CPU:
        return "Fallback CPU";
    default:
        return "Unknown";
    }
}

std::string CGPURenderingManager::GetGPUInfoString() const
{
    const auto& gpuInfo = gGPUDetector->GetGPUInfo();
    std::string result = "GPU: " + gpuInfo.Name + 
                         " | Mode: " + GetRenderingModeString();
    return result;
}

bool CGPURenderingManager::SupportsDeferred() const
{
    return m_RenderingMode == RenderingMode::NVIDIA_OPTIMIZED ||
           m_RenderingMode == RenderingMode::AMD_COMPATIBLE;
}

bool CGPURenderingManager::SupportsCompute() const
{
    const auto& gpuInfo = gGPUDetector->GetGPUInfo();
    return gpuInfo.SupportsCompute;
}

bool CGPURenderingManager::SupportsTessellation() const
{
    const auto& gpuInfo = gGPUDetector->GetGPUInfo();
    return gpuInfo.SupportsTessellation;
}

bool CGPURenderingManager::SupportsInstancing() const
{
    const auto& gpuInfo = gGPUDetector->GetGPUInfo();
    return gpuInfo.SupportsInstancing;
}

bool CGPURenderingManager::SupportsAsyncCompute() const
{
    return m_RenderingMode == RenderingMode::NVIDIA_OPTIMIZED;
}

void CGPURenderingManager::PrintDetectionResults()
{
    std::cout << "\n??????????????????????????????????????????" << std::endl;
    std::cout << "?      RENDERING MODE ACTIVATED         ?" << std::endl;
    std::cout << "??????????????????????????????????????????\n" << std::endl;

    const auto& gpuInfo = gGPUDetector->GetGPUInfo();

    std::cout << "?? GPU SELECTION:" << std::endl;
    std::cout << "  Detected: " << gGPUDetector->GetGPUTypeString() << std::endl;
    std::cout << "  Model:    " << gpuInfo.Name << std::endl;

    std::cout << "\n??  RENDERING MODE:" << std::endl;
    std::cout << "  Mode:     " << GetRenderingModeString() << std::endl;

    std::cout << "\n? ENABLED FEATURES:" << std::endl;
    std::cout << "  Deferred:    " << (SupportsDeferred() ? "?" : "?") << std::endl;
    std::cout << "  Compute:     " << (SupportsCompute() ? "?" : "?") << std::endl;
    std::cout << "  Tessellation:" << (SupportsTessellation() ? "?" : "?") << std::endl;
    std::cout << "  Instancing:  " << (SupportsInstancing() ? "?" : "?") << std::endl;
    std::cout << "  AsyncCompute:" << (SupportsAsyncCompute() ? "?" : "?") << std::endl;

    std::cout << "\n????????????????????????????????????????\n" << std::endl;
}

void CGPURenderingManager::LogToFile(const char* filename)
{
    gGPUDetector->LogGPUInfo(filename);
}

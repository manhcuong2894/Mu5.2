#include "stdafx.h"
#include "CGPUDetector.h"
#include <GL/glew.h>
#include <iostream>
#include <cstring>
#include <fstream>

CGPUDetector* CGPUDetector::m_pInstance = nullptr;

CGPUDetector::CGPUDetector()
    : m_bDetected(false)
{
    memset(&m_DetectedGPU, 0, sizeof(m_DetectedGPU));
    m_DetectedGPU.Type = GPUType::UNKNOWN;
}

CGPUDetector::~CGPUDetector()
{
}

CGPUDetector* CGPUDetector::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CGPUDetector();
    }
    return m_pInstance;
}

bool CGPUDetector::DetectGPU()
{
    if (m_bDetected)
        return true;

    // Verificar se GLEW está inicializado
    if (glewGetString(GLEW_VERSION) == nullptr)
    {
        std::cerr << "[GPU Detector] GLEW não inicializado!" << std::endl;
        return false;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Detectando GPU do Sistema..." << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Query GPU info
    QueryGPUInfo();

    // Identificar vendor
    IdentifyVendor();

    // Query capabilities
    QueryCapabilities();

    // Detect features
    DetectFeatures();

    m_bDetected = true;

    PrintGPUInfo();

    return true;
}

void CGPUDetector::QueryGPUInfo()
{
    // Obter informações básicas
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* version = (const char*)glGetString(GL_VERSION);

    if (renderer) m_DetectedGPU.Renderer = renderer;
    if (vendor) m_DetectedGPU.Vendor = vendor;
    if (version) m_DetectedGPU.Version = version;

    // Extrair nome da GPU
    if (renderer)
    {
        m_DetectedGPU.Name = renderer;
    }
}

void CGPUDetector::IdentifyVendor()
{
    std::string renderer = m_DetectedGPU.Renderer;
    std::string vendor = m_DetectedGPU.Vendor;

    // Converter para minúsculas para comparação
    std::transform(renderer.begin(), renderer.end(), renderer.begin(), ::tolower);
    std::transform(vendor.begin(), vendor.end(), vendor.begin(), ::tolower);

    // Detectar NVIDIA
    if (vendor.find("nvidia") != std::string::npos ||
        renderer.find("nvidia") != std::string::npos ||
        renderer.find("geforce") != std::string::npos ||
        renderer.find("tesla") != std::string::npos)
    {
        m_DetectedGPU.Type = GPUType::NVIDIA;
        std::cout << "[GPU Detector] ? NVIDIA detectada!" << std::endl;
        return;
    }

    // Detectar AMD
    if (vendor.find("amd") != std::string::npos ||
        renderer.find("radeon") != std::string::npos ||
        renderer.find("amd") != std::string::npos ||
        renderer.find("firepro") != std::string::npos ||
        renderer.find("ryzen") != std::string::npos)
    {
        m_DetectedGPU.Type = GPUType::AMD;
        std::cout << "[GPU Detector] ? AMD detectada!" << std::endl;
        return;
    }

    // Detectar Intel
    if (vendor.find("intel") != std::string::npos ||
        renderer.find("intel") != std::string::npos ||
        renderer.find("iris") != std::string::npos ||
        renderer.find("uhd") != std::string::npos)
    {
        m_DetectedGPU.Type = GPUType::INTEL;
        std::cout << "[GPU Detector] ? Intel detectada!" << std::endl;
        return;
    }

    // Desconhecida
    m_DetectedGPU.Type = GPUType::OTHER;
    std::cout << "[GPU Detector] ??  GPU desconhecida: " << m_DetectedGPU.Name << std::endl;
}

void CGPUDetector::QueryCapabilities()
{
    // Texture size
    GLint maxTexSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    m_DetectedGPU.MaxTextureSize = maxTexSize;

    // Texture units
    GLint maxTexUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexUnits);
    m_DetectedGPU.MaxTextureUnits = maxTexUnits;

    // Compute work group size
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &m_DetectedGPU.MaxComputeWorkGroupSize[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &m_DetectedGPU.MaxComputeWorkGroupSize[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &m_DetectedGPU.MaxComputeWorkGroupSize[2]);
}

void CGPUDetector::DetectFeatures()
{
    // Verificar extensões
    m_DetectedGPU.SupportsCompute = glewIsSupported("GL_ARB_compute_shader");
    m_DetectedGPU.SupportsTessellation = glewIsSupported("GL_ARB_tessellation_shader");
    m_DetectedGPU.SupportsGeometryShader = glewIsSupported("GL_ARB_geometry_shader4");
    m_DetectedGPU.SupportsInstancing = glewIsSupported("GL_ARB_instanced_arrays");
    m_DetectedGPU.SupportsDeferred = m_DetectedGPU.MaxTextureUnits >= 8;
}

std::string CGPUDetector::GetGPUTypeString() const
{
    switch (m_DetectedGPU.Type)
    {
    case GPUType::NVIDIA:
        return "NVIDIA";
    case GPUType::AMD:
        return "AMD";
    case GPUType::INTEL:
        return "Intel";
    case GPUType::OTHER:
        return "Other";
    default:
        return "Unknown";
    }
}

std::string CGPUDetector::GetGPUTypeAsText() const
{
    switch (m_DetectedGPU.Type)
    {
    case GPUType::NVIDIA:
        return "GeForce / Tesla";
    case GPUType::AMD:
        return "Radeon / FirePro";
    case GPUType::INTEL:
        return "Iris / UHD Graphics";
    case GPUType::OTHER:
        return "Other Vendor";
    default:
        return "Unknown";
    }
}

void CGPUDetector::PrintGPUInfo()
{
    std::cout << "\n??????????????????????????????????????????" << std::endl;
    std::cout << "?       GPU DETECTION RESULTS           ?" << std::endl;
    std::cout << "??????????????????????????????????????????\n" << std::endl;

    std::cout << "?? GPU DETECTED:" << std::endl;
    std::cout << "  Type:        " << GetGPUTypeString() << std::endl;
    std::cout << "  Name:        " << m_DetectedGPU.Name << std::endl;
    std::cout << "  Vendor:      " << m_DetectedGPU.Vendor << std::endl;
    std::cout << "  Renderer:    " << m_DetectedGPU.Renderer << std::endl;

    std::cout << "\n?? CAPABILITIES:" << std::endl;
    std::cout << "  Max Texture Size:      " << m_DetectedGPU.MaxTextureSize << std::endl;
    std::cout << "  Max Texture Units:     " << m_DetectedGPU.MaxTextureUnits << std::endl;
    std::cout << "  Compute Shader:        " << (m_DetectedGPU.SupportsCompute ? "? YES" : "? NO") << std::endl;
    std::cout << "  Tessellation:          " << (m_DetectedGPU.SupportsTessellation ? "? YES" : "? NO") << std::endl;
    std::cout << "  Geometry Shader:       " << (m_DetectedGPU.SupportsGeometryShader ? "? YES" : "? NO") << std::endl;
    std::cout << "  Instancing:            " << (m_DetectedGPU.SupportsInstancing ? "? YES" : "? NO") << std::endl;
    std::cout << "  Deferred Rendering:    " << (m_DetectedGPU.SupportsDeferred ? "? YES" : "? NO") << std::endl;

    std::cout << "\n?? OPTIMAL RENDERING:" << std::endl;
    if (SupportsOptimalRendering())
    {
        std::cout << "  ? Sistema NVIDIA otimizado ativado!" << std::endl;
    }
    else if (CanFallbackToDeferred())
    {
        std::cout << "  ??  Usando Deferred Rendering (compatível)" << std::endl;
    }
    else if (CanFallbackToForward())
    {
        std::cout << "  ??  Usando Forward Rendering (modo compatibilidade)" << std::endl;
    }
    else
    {
        std::cout << "  ? Renderização mínima (CPU fallback)" << std::endl;
    }

    std::cout << "\n????????????????????????????????????????\n" << std::endl;
}

void CGPUDetector::LogGPUInfo(const char* filename)
{
    std::ofstream file(filename, std::ios::app);
    
    if (!file.is_open())
        return;

    file << "=== GPU DETECTION LOG ===" << std::endl;
    file << "GPU Type:       " << GetGPUTypeString() << std::endl;
    file << "GPU Name:       " << m_DetectedGPU.Name << std::endl;
    file << "Vendor:         " << m_DetectedGPU.Vendor << std::endl;
    file << "Renderer:       " << m_DetectedGPU.Renderer << std::endl;
    file << "Max Texture:    " << m_DetectedGPU.MaxTextureSize << std::endl;
    file << "Compute:        " << (m_DetectedGPU.SupportsCompute ? "YES" : "NO") << std::endl;
    file << "Deferred:       " << (m_DetectedGPU.SupportsDeferred ? "YES" : "NO") << std::endl;
    file << std::endl;

    file.close();
}

bool CGPUDetector::SupportsOptimalRendering() const
{
    // GPU suporta renderização otimizada
    return m_DetectedGPU.SupportsCompute && 
           m_DetectedGPU.SupportsDeferred && 
           m_DetectedGPU.SupportsInstancing;
}

bool CGPUDetector::CanFallbackToDeferred() const
{
    // GPU pode usar deferred rendering como fallback
    return m_DetectedGPU.SupportsDeferred && 
           m_DetectedGPU.MaxTextureUnits >= 8;
}

bool CGPUDetector::CanFallbackToForward() const
{
    // GPU pode usar forward rendering básico
    return m_DetectedGPU.MaxTextureSize >= 2048;
}

bool CGPUDetector::ContainsString(const std::string& text, const std::string& search) const
{
    return text.find(search) != std::string::npos;
}

size_t CGPUDetector::ExtractMemoryFromString(const std::string& text) const
{
    // Extrair memória da string (simplificado)
    return 0;
}

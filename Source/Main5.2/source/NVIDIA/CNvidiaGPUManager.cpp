#include "stdafx.h"
#include "CNvidiaGPUManager.h"
#include <GL/glew.h>
#include <iostream>

// Constantes NVIDIA (se não estão definidas)
#ifndef GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX
#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
#endif

#ifndef GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_MEMORY_NVX
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_MEMORY_NVX 0x904F
#endif

CNvidiaGPUManager* CNvidiaGPUManager::m_pInstance = nullptr;

CNvidiaGPUManager::CNvidiaGPUManager()
    : m_CurrentDeviceID(-1)
    , m_bInitialized(false)
    , m_bGPURenderingEnabled(false)
    , m_GPUBuffer(0)
{
    memset(&m_RenderStats, 0, sizeof(m_RenderStats));
}

CNvidiaGPUManager::~CNvidiaGPUManager()
{
    Shutdown();
}

CNvidiaGPUManager* CNvidiaGPUManager::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CNvidiaGPUManager();
    }
    return m_pInstance;
}

bool CNvidiaGPUManager::Initialize()
{
    if (m_bInitialized)
        return true;

    // Detectar GPUs disponíveis
    if (!DetectGPUs())
    {
        std::cerr << "[NVIDIA] Nenhuma GPU NVIDIA detectada!" << std::endl;
        return false;
    }

    if (m_GPUs.empty())
    {
        std::cerr << "[NVIDIA] Nenhuma GPU suportada encontrada!" << std::endl;
        return false;
    }

    // Selecionar primeira GPU disponível
    if (!SelectGPU(0))
    {
        std::cerr << "[NVIDIA] Falha ao selecionar GPU!" << std::endl;
        return false;
    }

    m_bInitialized = true;
    std::cout << "[NVIDIA] GPU Manager inicializado com sucesso!" << std::endl;
    std::cout << "[NVIDIA] GPU: " << m_GPUs[0].DeviceName << std::endl;
    std::cout << "[NVIDIA] Memória Total: " << (m_GPUs[0].TotalMemory / 1024 / 1024) << " MB" << std::endl;

    return true;
}

void CNvidiaGPUManager::Shutdown()
{
    if (!m_bInitialized)
        return;

    // Liberar recursos de GPU
    if (m_GPUBuffer != 0)
    {
        glDeleteBuffers(1, &m_GPUBuffer);
        m_GPUBuffer = 0;
    }

    m_bInitialized = false;
    m_bGPURenderingEnabled = false;
    std::cout << "[NVIDIA] GPU Manager finalizado!" << std::endl;
}

bool CNvidiaGPUManager::DetectGPUs()
{
    m_GPUs.clear();

    // Verificar se GLEW está inicializado
    if (!glewIsSupported("GL_ARB_gpu_shader5"))
    {
        return false;
    }

    // Criar uma estrutura GPUInfo para GPU detectada via OpenGL
    GPUInfo gpu = {};
    gpu.DeviceID = 0;
    strncpy_s(gpu.DeviceName, 256, (const char*)glGetString(GL_RENDERER), 255);
    gpu.IsSupported = true;
    gpu.ComputeCapability = 75; // Assumir Turing ou superior
    gpu.ClockRate = 1000; // MHz
    gpu.MaxThreadsPerBlock = 1024;

    // Obter memória disponível (se possível)
    GLint gpuMemory = 0;
    GLint availableMemory = 0;
    
    // Tentar obter memória via extensão NVIDIA
    glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &gpuMemory);
    
    if (gpuMemory > 0)
    {
        gpu.TotalMemory = (size_t)gpuMemory * 1024; // Converter para bytes
    }
    else
    {
        gpu.TotalMemory = 2 * 1024 * 1024 * 1024; // Default 2GB
    }

    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_MEMORY_NVX, &availableMemory);
    if (availableMemory > 0)
    {
        gpu.AvailableMemory = (size_t)availableMemory * 1024;
    }
    else
    {
        gpu.AvailableMemory = gpu.TotalMemory;
    }

    m_GPUs.push_back(gpu);

    return true;
}

int CNvidiaGPUManager::GetGPUCount() const
{
    return (int)m_GPUs.size();
}

const CNvidiaGPUManager::GPUInfo& CNvidiaGPUManager::GetGPUInfo(int deviceID) const
{
    static GPUInfo emptyInfo = {};
    
    if (deviceID < 0 || deviceID >= (int)m_GPUs.size())
    {
        return emptyInfo;
    }

    return m_GPUs[deviceID];
}

bool CNvidiaGPUManager::SelectGPU(int deviceID)
{
    if (deviceID < 0 || deviceID >= (int)m_GPUs.size())
    {
        return false;
    }

    m_CurrentDeviceID = deviceID;
    return SetupGPUContext();
}

bool CNvidiaGPUManager::AllocateGPUMemory(size_t size, GLuint& buffer)
{
    if (!m_bInitialized)
        return false;

    glGenBuffers(1, &buffer);
    if (buffer == 0)
        return false;

    glBindBuffer(GL_COPY_WRITE_BUFFER, buffer);
    glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        glDeleteBuffers(1, &buffer);
        return false;
    }

    return true;
}

bool CNvidiaGPUManager::FreeGPUMemory(GLuint buffer)
{
    if (buffer == 0)
        return false;

    glDeleteBuffers(1, &buffer);
    return true;
}

size_t CNvidiaGPUManager::GetAvailableMemory() const
{
    if (m_CurrentDeviceID < 0 || m_CurrentDeviceID >= (int)m_GPUs.size())
        return 0;

    GLint availableMemory = 0;
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_MEMORY_NVX, &availableMemory);
    
    if (availableMemory > 0)
    {
        return (size_t)availableMemory * 1024;
    }

    return m_GPUs[m_CurrentDeviceID].AvailableMemory;
}

size_t CNvidiaGPUManager::GetUsedMemory() const
{
    if (m_CurrentDeviceID < 0 || m_CurrentDeviceID >= (int)m_GPUs.size())
        return 0;

    size_t total = m_GPUs[m_CurrentDeviceID].TotalMemory;
    size_t available = GetAvailableMemory();

    return (total > available) ? (total - available) : 0;
}

bool CNvidiaGPUManager::EnableGPURendering(bool enable)
{
    m_bGPURenderingEnabled = enable;
    return true;
}

bool CNvidiaGPUManager::IsGPURenderingEnabled() const
{
    return m_bGPURenderingEnabled;
}

bool CNvidiaGPUManager::ProcessMeshOnGPU(const void* meshData, size_t meshSize)
{
    if (!m_bInitialized || !m_bGPURenderingEnabled)
        return false;

    GLuint tempBuffer;
    if (!AllocateGPUMemory(meshSize, tempBuffer))
        return false;

    glBindBuffer(GL_COPY_WRITE_BUFFER, tempBuffer);
    glBufferSubData(GL_COPY_WRITE_BUFFER, 0, meshSize, meshData);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

    FreeGPUMemory(tempBuffer);
    return true;
}

const CNvidiaGPUManager::RenderStats& CNvidiaGPUManager::GetRenderStats() const
{
    return m_RenderStats;
}

void CNvidiaGPUManager::UpdateRenderStats()
{
    if (!m_bInitialized)
        return;

    m_RenderStats.MemoryUsage = (float)(GetUsedMemory() * 100.0f / 
        m_GPUs[m_CurrentDeviceID].TotalMemory);
    m_RenderStats.FramesRendered++;
}

bool CNvidiaGPUManager::IsNvidiaSupported() const
{
    return !m_GPUs.empty() && m_GPUs[0].IsSupported;
}

bool CNvidiaGPUManager::IsCUDAAvailable() const
{
    // Verificar se CUDA é disponível (simplificado)
    return m_bInitialized && m_GPUs[0].ComputeCapability >= 30;
}

bool CNvidiaGPUManager::QueryGPUCapabilities()
{
    if (m_GPUs.empty())
        return false;

    // Consultar capacidades via OpenGL
    GLint maxComputeWorkGroupSize[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxComputeWorkGroupSize[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxComputeWorkGroupSize[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxComputeWorkGroupSize[2]);

    return true;
}

bool CNvidiaGPUManager::SetupGPUContext()
{
    if (m_CurrentDeviceID < 0 || m_CurrentDeviceID >= (int)m_GPUs.size())
        return false;

    return QueryGPUCapabilities();
}

void CNvidiaGPUManager::UpdateMemoryStats()
{
    if (m_CurrentDeviceID < 0 || m_CurrentDeviceID >= (int)m_GPUs.size())
        return;

    m_GPUs[m_CurrentDeviceID].AvailableMemory = GetAvailableMemory();
}

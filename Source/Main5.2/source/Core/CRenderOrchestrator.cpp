#include "stdafx.h"
#include "CRenderOrchestrator.h"
#include "CRenderPipeline.h"
#include "CPerformanceMonitor.h"
#include <GL/glew.h>
#include <chrono>
#include <algorithm>

CRenderOrchestrator* CRenderOrchestrator::m_pInstance = nullptr;

CRenderOrchestrator::CRenderOrchestrator()
    : m_bInitialized(false),
      m_bGPUAccelerationActive(false),
      m_bFPSLocked(false),
      m_bProfilingEnabled(false),
      m_DetectedGPU(GPUVendor::FALLBACK),
      m_PreferredGPU(GPUVendor::NVIDIA),
      m_FrameCount(0),
      m_LastFrameTime(0.0),
      m_FrameStartTime(0.0),
      m_TargetFrameTime(10000)  // 100 FPS = 10ms
{
}

CRenderOrchestrator::~CRenderOrchestrator()
{
    Shutdown();
}

CRenderOrchestrator* CRenderOrchestrator::Instance()
{
    if (!m_pInstance)
        m_pInstance = new CRenderOrchestrator();
    return m_pInstance;
}

bool CRenderOrchestrator::Initialize()
{
    if (m_bInitialized)
        return true;

    // Detectar GPU
    DetectGPU();

    // Inicializar pipeline
    if (!gRenderPipeline->Initialize())
        return false;

    // Inicializar monitor
    if (!gPerformanceMonitor->Initialize())
        return false;

    // Criar GPU queries
    glGenQueries(1, &m_GPUTimeQuery);
    glGenQueries(1, &m_PrimitiveQuery);

    m_bInitialized = true;
    m_bGPUAccelerationActive = true;

    return true;
}

void CRenderOrchestrator::Shutdown()
{
    if (!m_bInitialized)
        return;

    glDeleteQueries(1, &m_GPUTimeQuery);
    glDeleteQueries(1, &m_PrimitiveQuery);

    gRenderPipeline->Shutdown();
    m_bInitialized = false;
}

void CRenderOrchestrator::SubmitBatch(const RenderBatch& batch)
{
    if (!m_bInitialized)
        return;

    m_BatchQueue.push_back(batch);

    if (m_Config.EnableBatching && m_BatchQueue.size() >= m_Config.MaxBatchSize)
    {
        OptimizeBatches();
    }
}

void CRenderOrchestrator::ClearBatches()
{
    m_BatchQueue.clear();
    m_SortedBatches.clear();
}

void CRenderOrchestrator::OptimizeBatches()
{
    // Separar batches por prioridade
    for (const auto& batch : m_BatchQueue)
    {
        m_SortedBatches[batch.Priority].push_back(batch);
    }

    // Otimizar para GPU específica
    if (m_bGPUAccelerationActive)
    {
        OptimizeForGPU(m_DetectedGPU);
    }

    m_BatchQueue.clear();
}

void CRenderOrchestrator::BeginFrame()
{
    gPerformanceMonitor->StartFrame();
    m_FrameStartTime = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void CRenderOrchestrator::RenderFrame()
{
    if (!m_bInitialized)
        return;

    // Sincronizar estados
    SyncShaderState();
    SyncColorSystem();
    SyncGPUState();

    // Renderizar por prioridade
    m_FrameStats.DrawCalls = 0;
    m_FrameStats.TrianglesRendered = 0;
    m_FrameStats.BatchesSubmitted = 0;

    for (auto& [priority, batches] : m_SortedBatches)
    {
        for (const auto& batch : batches)
        {
            glUseProgram(batch.ShaderProgram);
            glBindVertexArray(batch.VAO);

            if (batch.Texture != 0)
            {
                glBindTexture(GL_TEXTURE_2D, batch.Texture);
            }

            if (batch.UseInstancing && batch.InstanceCount > 1)
            {
                glDrawElementsInstanced(GL_TRIANGLES, batch.IndexCount, GL_UNSIGNED_INT, 0, batch.InstanceCount);
            }
            else
            {
                glDrawElements(GL_TRIANGLES, batch.IndexCount, GL_UNSIGNED_INT, 0);
            }

            m_FrameStats.DrawCalls++;
            m_FrameStats.TrianglesRendered += batch.IndexCount / 3;
            m_FrameStats.BatchesSubmitted++;
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);

    // Renderizar Scaleform
    SyncScaleformState();
}

void CRenderOrchestrator::EndFrame()
{
    gPerformanceMonitor->EndFrame();
    MeasureFrameTime();
    UpdateFrameStats();

    if (m_bFPSLocked)
    {
        EnforceFrameRate();
    }

    m_FrameCount++;
}

void CRenderOrchestrator::Present()
{
    glFlush();
    glFinish();
}

void CRenderOrchestrator::SyncShaderState()
{
    // Sincronizar estado de shaders com GPU
    // CShaderGL já mantém estado próprio
}

void CRenderOrchestrator::SyncScaleformState()
{
    // Sincronizar renderização Scaleform
    glm::mat4 ortho = glm::mat4(1.0f);
    // gScaleformSystem->RenderAllMovies(ortho, glm::mat4(1.0f));
}

void CRenderOrchestrator::SyncColorSystem()
{
    // Sincronizar cores com CColorSystem
    // gColorSystem já está sincronizado nos shaders
}

void CRenderOrchestrator::SyncGPUState()
{
    // Sincronizar estado GPU específico
    switch (m_DetectedGPU)
    {
        case GPUVendor::NVIDIA:
            // Otimizações NVIDIA
            break;
        case GPUVendor::AMD:
            // Otimizações AMD
            break;
        case GPUVendor::INTEL:
            // Otimizações Intel
            break;
        default:
            break;
    }
}

bool CRenderOrchestrator::SetPreferredGPU(GPUVendor vendor)
{
    m_PreferredGPU = vendor;
    return true;
}

bool CRenderOrchestrator::UseGPUAcceleration(bool enable)
{
    m_bGPUAccelerationActive = enable;
    return true;
}

void CRenderOrchestrator::LockFPS(uint32_t targetFPS)
{
    m_Config.TargetFPS = targetFPS;
    m_TargetFrameTime = (1000000 / targetFPS);  // microsegundos
    m_bFPSLocked = true;
}

void CRenderOrchestrator::UnlockFPS()
{
    m_bFPSLocked = false;
}

float CRenderOrchestrator::GetAverageFPS(uint32_t samples) const
{
    if (m_FPSHistory.empty())
        return 0.0f;

    uint32_t count = std::min(samples, (uint32_t)m_FPSHistory.size());
    float sum = 0.0f;

    for (uint32_t i = 0; i < count; i++)
    {
        sum += m_FPSHistory[m_FPSHistory.size() - 1 - i];
    }

    return sum / count;
}

void CRenderOrchestrator::SetConfig(const Config& config)
{
    m_Config = config;
}

void CRenderOrchestrator::PrintRenderStats() const
{
}

void CRenderOrchestrator::PrintGPUInfo() const
{
}

void CRenderOrchestrator::ToggleProfiling(bool enable)
{
    m_bProfilingEnabled = enable;
}

// Métodos privados

void CRenderOrchestrator::DetectGPU()
{
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    
    if (vendor)
    {
        std::string vendorStr(vendor);
        
        if (vendorStr.find("NVIDIA") != std::string::npos)
        {
            m_DetectedGPU = GPUVendor::NVIDIA;
        }
        else if (vendorStr.find("AMD") != std::string::npos || 
                 vendorStr.find("ATI") != std::string::npos)
        {
            m_DetectedGPU = GPUVendor::AMD;
        }
        else if (vendorStr.find("Intel") != std::string::npos)
        {
            m_DetectedGPU = GPUVendor::INTEL;
        }
        else
        {
            m_DetectedGPU = GPUVendor::FALLBACK;
        }
    }
}

void CRenderOrchestrator::SelectRenderPath()
{
    // Selecionar caminho de renderização baseado em GPU
}

void CRenderOrchestrator::SubmitBatchesToGPU()
{
    // Submeter batches otimizados para GPU
}

void CRenderOrchestrator::OptimizeForGPU(GPUVendor vendor)
{
    switch (vendor)
    {
        case GPUVendor::NVIDIA:
            // Usar CUDA, async compute, etc
            m_Config.UseAsyncCompute = true;
            break;
        case GPUVendor::AMD:
            // Usar ACE, LDS, etc
            m_Config.EnableTessellation = true;
            break;
        case GPUVendor::INTEL:
            // Otimizações Intel
            break;
        default:
            break;
    }
}

void CRenderOrchestrator::MeasureFrameTime()
{
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    double frameTime = (now - m_FrameStartTime) / 1000.0;  // converter para ms
    m_LastFrameTime = frameTime;

    float fps = 1000.0f / frameTime;
    m_FrameStats.FPS = fps;
    m_FrameStats.FrameTime = frameTime;
    m_FPSHistory.push_back(fps);

    if (m_FPSHistory.size() > 300)  // Manter últimas 300 frames
    {
        m_FPSHistory.erase(m_FPSHistory.begin());
    }
}

void CRenderOrchestrator::EnforceFrameRate()
{
    // Aguardar para atingir frame rate alvo
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto elapsed = now - m_FrameStartTime;

    if (elapsed < m_TargetFrameTime)
    {
        double sleepTime = m_TargetFrameTime - elapsed;
        std::this_thread::sleep_for(std::chrono::microseconds((long long)sleepTime));
    }
}

void CRenderOrchestrator::UpdateFrameStats()
{
    // Atualizar estatísticas
    gPerformanceMonitor->PrintMetrics();
}

#include "stdafx.h"
#include "CIntelRenderSystem.h"
#include <iostream>

CIntelRenderSystem* CIntelRenderSystem::m_pInstance = nullptr;

CIntelRenderSystem::CIntelRenderSystem()
    : m_ForwardShader(0)
    , m_LightCullingShader(0)
    , m_LightBuffer(0)
    , m_LightCullingBuffer(0)
    , m_bInitialized(false)
{
    memset(&m_Stats, 0, sizeof(m_Stats));
}

CIntelRenderSystem::~CIntelRenderSystem()
{
    Shutdown();
}

CIntelRenderSystem* CIntelRenderSystem::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CIntelRenderSystem();
    }
    return m_pInstance;
}

CIntelRenderSystem::RenderConfig CIntelRenderSystem::GetDefaultConfig()
{
    RenderConfig config = {};
    config.EnableForward = true;
    config.EnableInstancing = true;
    config.EnableEUOptimization = true;
    config.EnablePredicates = true;
    config.EnableEarlyZ = true;
    config.DetailLevel = 0.6f;
    config.MaxLightsPerObject = 8;
    config.ShadowResolution = 1024;
    config.EnableGPUCulling = true;
    config.EnableLightCulling = true;
    config.ShowDebugInfo = false;
    config.WireframeMode = false;
    return config;
}

bool CIntelRenderSystem::Initialize(const RenderConfig& config)
{
    if (m_bInitialized)
        return true;

    m_Config = config;

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Intel Render System Inicialização" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Compilar shaders
    if (!CompileShaders())
    {
        std::cerr << "[Intel Render] Falha ao compilar shaders!" << std::endl;
        return false;
    }

    m_bInitialized = true;

    std::cout << "[Intel Render] ? Inicializado com sucesso!" << std::endl;
    std::cout << "[Intel Render] Forward: " << (m_Config.EnableForward ? "SIM" : "NÃO") << std::endl;
    std::cout << "[Intel Render] EU Opt: " << (m_Config.EnableEUOptimization ? "SIM" : "NÃO") << std::endl;
    std::cout << "[Intel Render] Max Lights: " << m_Config.MaxLightsPerObject << std::endl;

    return true;
}

void CIntelRenderSystem::Shutdown()
{
    if (!m_bInitialized)
        return;

    if (m_ForwardShader != 0)
    {
        glDeleteProgram(m_ForwardShader);
        m_ForwardShader = 0;
    }

    if (m_LightCullingShader != 0)
    {
        glDeleteProgram(m_LightCullingShader);
        m_LightCullingShader = 0;
    }

    if (m_LightBuffer != 0)
    {
        glDeleteBuffers(1, &m_LightBuffer);
        m_LightBuffer = 0;
    }

    m_bInitialized = false;
    std::cout << "[Intel Render] Finalizado!" << std::endl;
}

void CIntelRenderSystem::BeginFrame()
{
    if (!m_bInitialized)
        return;

    ResetStats();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CIntelRenderSystem::RenderObject(GLuint vao, GLuint shader, int indexCount)
{
    if (!m_bInitialized)
        return;

    glUseProgram(shader);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    m_Stats.DrawCalls++;
}

void CIntelRenderSystem::EndFrame()
{
    if (!m_bInitialized)
        return;

    glUseProgram(0);
}

void CIntelRenderSystem::ResetStats()
{
    m_Stats.DrawCalls = 0;
    m_Stats.TrianglesRendered = 0;
    m_Stats.LightsPerFrame = 0;
    m_Stats.FrameTimeMS = 0.0f;
}

void CIntelRenderSystem::PrintStats()
{
    std::cout << "\n[Intel Render] Stats:" << std::endl;
    std::cout << "  Draw Calls: " << m_Stats.DrawCalls << std::endl;
    std::cout << "  Triangles: " << m_Stats.TrianglesRendered << std::endl;
    std::cout << "  Lights: " << m_Stats.LightsPerFrame << std::endl;
    std::cout << "  Frame Time: " << m_Stats.FrameTimeMS << " ms" << std::endl;
}

void CIntelRenderSystem::RenderDebugOverlay()
{
    if (!m_Config.ShowDebugInfo)
        return;

    PrintStats();
}

bool CIntelRenderSystem::CompileShaders()
{
    // Shaders serão compilados quando necessário
    return true;
}

void CIntelRenderSystem::UpdateLightCulling()
{
    // Implementar light culling específico para Intel
}

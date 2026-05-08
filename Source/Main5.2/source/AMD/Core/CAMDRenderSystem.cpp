#include "stdafx.h"
#include "CAMDRenderSystem.h"
#include <iostream>

CAMDRenderSystem* CAMDRenderSystem::m_pInstance = nullptr;

CAMDRenderSystem::CAMDRenderSystem()
    : m_DeferredShader(0)
    , m_LightShader(0)
    , m_CompositeShader(0)
    , m_DeferredFBO(0)
    , m_PositionTexture(0)
    , m_NormalTexture(0)
    , m_AlbedoTexture(0)
    , m_DepthTexture(0)
    , m_bInitialized(false)
{
    memset(&m_Stats, 0, sizeof(m_Stats));
}

CAMDRenderSystem::~CAMDRenderSystem()
{
    Shutdown();
}

CAMDRenderSystem* CAMDRenderSystem::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CAMDRenderSystem();
    }
    return m_pInstance;
}

CAMDRenderSystem::RenderConfig CAMDRenderSystem::GetDefaultConfig()
{
    RenderConfig config = {};
    config.EnableDeferred = true;
    config.EnableInstancing = true;
    config.EnableWaveOptimization = true;
    config.EnableLDSUsage = true;
    config.EnablePredicates = true;
    config.DetailLevel = 0.8f;
    config.MaxLights = 32;
    config.ShadowResolution = 2048;
    config.EnableGPUCulling = true;
    config.EnableAsyncCompute = false;
    config.ShowDebugInfo = false;
    config.WireframeMode = false;
    return config;
}

bool CAMDRenderSystem::Initialize(const RenderConfig& config)
{
    if (m_bInitialized)
        return true;

    m_Config = config;

    std::cout << "\n========================================" << std::endl;
    std::cout << "  AMD Render System Inicialização" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Compilar shaders
    if (!CompileShaders())
    {
        std::cerr << "[AMD Render] Falha ao compilar shaders!" << std::endl;
        return false;
    }

    // Criar framebuffers
    if (!CreateFramebuffers())
    {
        std::cerr << "[AMD Render] Falha ao criar framebuffers!" << std::endl;
        return false;
    }

    m_bInitialized = true;

    std::cout << "[AMD Render] ? Inicializado com sucesso!" << std::endl;
    std::cout << "[AMD Render] Deferred: " << (m_Config.EnableDeferred ? "SIM" : "NÃO") << std::endl;
    std::cout << "[AMD Render] Wave Opt: " << (m_Config.EnableWaveOptimization ? "SIM" : "NÃO") << std::endl;
    std::cout << "[AMD Render] Max Lights: " << m_Config.MaxLights << std::endl;

    return true;
}

void CAMDRenderSystem::Shutdown()
{
    if (!m_bInitialized)
        return;

    CleanupFramebuffers();

    if (m_DeferredShader != 0)
    {
        glDeleteProgram(m_DeferredShader);
        m_DeferredShader = 0;
    }

    if (m_LightShader != 0)
    {
        glDeleteProgram(m_LightShader);
        m_LightShader = 0;
    }

    if (m_CompositeShader != 0)
    {
        glDeleteProgram(m_CompositeShader);
        m_CompositeShader = 0;
    }

    m_bInitialized = false;
    std::cout << "[AMD Render] Finalizado!" << std::endl;
}

void CAMDRenderSystem::BeginFrame()
{
    if (!m_bInitialized)
        return;

    ResetStats();

    if (m_Config.EnableDeferred)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_DeferredFBO);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

void CAMDRenderSystem::BeginDeferredPass()
{
    if (!m_Config.EnableDeferred)
        return;

    glUseProgram(m_DeferredShader);
}

void CAMDRenderSystem::RenderLightPass()
{
    if (!m_Config.EnableDeferred)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(m_LightShader);

    // Ativar texturas do deferred
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_PositionTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_NormalTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_AlbedoTexture);

    // Renderizar quad full-screen
    // ... implementação ...
}

void CAMDRenderSystem::EndFrame()
{
    if (!m_bInitialized)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}

void CAMDRenderSystem::ResetStats()
{
    m_Stats.DrawCalls = 0;
    m_Stats.TrianglesRendered = 0;
    m_Stats.FrameTimeMS = 0.0f;
    m_Stats.LightsRendered = 0;
}

void CAMDRenderSystem::PrintStats()
{
    std::cout << "\n[AMD Render] Stats:" << std::endl;
    std::cout << "  Draw Calls: " << m_Stats.DrawCalls << std::endl;
    std::cout << "  Triangles: " << m_Stats.TrianglesRendered << std::endl;
    std::cout << "  Lights: " << m_Stats.LightsRendered << std::endl;
    std::cout << "  Frame Time: " << m_Stats.FrameTimeMS << " ms" << std::endl;
}

void CAMDRenderSystem::RenderDebugOverlay()
{
    if (!m_Config.ShowDebugInfo)
        return;

    PrintStats();
}

bool CAMDRenderSystem::CompileShaders()
{
    // Shaders serão compilados quando necessário
    return true;
}

bool CAMDRenderSystem::CreateFramebuffers()
{
    // Criar framebuffer deferred
    glGenFramebuffers(1, &m_DeferredFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_DeferredFBO);

    // Criar texturas de saída
    glGenTextures(1, &m_PositionTexture);
    glGenTextures(1, &m_NormalTexture);
    glGenTextures(1, &m_AlbedoTexture);
    glGenTextures(1, &m_DepthTexture);

    // ... configuração de texturas ...

    return true;
}

void CAMDRenderSystem::CleanupFramebuffers()
{
    if (m_DeferredFBO != 0)
    {
        glDeleteFramebuffers(1, &m_DeferredFBO);
        m_DeferredFBO = 0;
    }

    if (m_PositionTexture != 0)
    {
        glDeleteTextures(1, &m_PositionTexture);
        m_PositionTexture = 0;
    }

    if (m_NormalTexture != 0)
    {
        glDeleteTextures(1, &m_NormalTexture);
        m_NormalTexture = 0;
    }

    if (m_AlbedoTexture != 0)
    {
        glDeleteTextures(1, &m_AlbedoTexture);
        m_AlbedoTexture = 0;
    }

    if (m_DepthTexture != 0)
    {
        glDeleteTextures(1, &m_DepthTexture);
        m_DepthTexture = 0;
    }
}

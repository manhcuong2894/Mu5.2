#include "stdafx.h"
#include "CFallbackRenderSystem.h"
#include <iostream>

CFallbackRenderSystem* CFallbackRenderSystem::m_pInstance = nullptr;

CFallbackRenderSystem::CFallbackRenderSystem()
    : m_BasicShader(0)
    , m_bInitialized(false)
{
    memset(&m_Stats, 0, sizeof(m_Stats));
}

CFallbackRenderSystem::~CFallbackRenderSystem()
{
    Shutdown();
}

CFallbackRenderSystem* CFallbackRenderSystem::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CFallbackRenderSystem();
    }
    return m_pInstance;
}

CFallbackRenderSystem::RenderConfig CFallbackRenderSystem::GetDefaultConfig()
{
    RenderConfig config = {};
    config.EnableBasicLighting = true;
    config.EnableShadows = false;
    config.EnableTexturing = true;
    config.DetailLevel = 0.3f;
    config.MaxLights = 2;
    config.MaxTextureSize = 1024;
    config.ShowDebugInfo = false;
    return config;
}

bool CFallbackRenderSystem::Initialize(const RenderConfig& config)
{
    if (m_bInitialized)
        return true;

    m_Config = config;

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Fallback Render System Inicialização" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Compilar shader básico
    if (!CompileBasicShader())
    {
        std::cerr << "[Fallback Render] Falha ao compilar shader!" << std::endl;
        return false;
    }

    m_bInitialized = true;

    std::cout << "[Fallback Render] ? Inicializado com sucesso!" << std::endl;
    std::cout << "[Fallback Render] Iluminação Básica: " << (m_Config.EnableBasicLighting ? "SIM" : "NÃO") << std::endl;
    std::cout << "[Fallback Render] Sombras: " << (m_Config.EnableShadows ? "SIM" : "NÃO") << std::endl;
    std::cout << "[Fallback Render] Max Lights: " << m_Config.MaxLights << std::endl;

    return true;
}

void CFallbackRenderSystem::Shutdown()
{
    if (!m_bInitialized)
        return;

    if (m_BasicShader != 0)
    {
        glDeleteProgram(m_BasicShader);
        m_BasicShader = 0;
    }

    m_bInitialized = false;
    std::cout << "[Fallback Render] Finalizado!" << std::endl;
}

void CFallbackRenderSystem::BeginFrame()
{
    if (!m_bInitialized)
        return;

    ResetStats();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CFallbackRenderSystem::RenderObject(GLuint vao, GLuint shader, int indexCount)
{
    if (!m_bInitialized)
        return;

    glUseProgram(shader ? shader : m_BasicShader);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    m_Stats.DrawCalls++;
}

void CFallbackRenderSystem::EndFrame()
{
    if (!m_bInitialized)
        return;

    glUseProgram(0);
}

void CFallbackRenderSystem::ResetStats()
{
    m_Stats.DrawCalls = 0;
    m_Stats.TrianglesRendered = 0;
    m_Stats.FrameTimeMS = 0.0f;
}

void CFallbackRenderSystem::PrintStats()
{
    std::cout << "\n[Fallback Render] Stats:" << std::endl;
    std::cout << "  Draw Calls: " << m_Stats.DrawCalls << std::endl;
    std::cout << "  Triangles: " << m_Stats.TrianglesRendered << std::endl;
    std::cout << "  Frame Time: " << m_Stats.FrameTimeMS << " ms" << std::endl;
}

bool CFallbackRenderSystem::CompileBasicShader()
{
    // Shader básico será compilado quando necessário
    return true;
}

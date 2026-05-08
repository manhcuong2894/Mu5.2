#include "stdafx.h"
#include "CRenderPipeline.h"
#include <GL/glew.h>

CRenderPipeline* CRenderPipeline::m_pInstance = nullptr;

CRenderPipeline::CRenderPipeline()
    : m_bInitialized(false),
      m_bDeferredLighting(true),
      m_GBufferFBO(0),
      m_NormalTexture(0),
      m_AlbedoTexture(0),
      m_DepthTexture(0),
      m_EmissiveTexture(0),
      m_PositionTexture(0),
      m_GeometryShader(0),
      m_LightingShader(0),
      m_CompositeShader(0),
      m_DrawCallCount(0),
      m_TriangleCount(0)
{
}

CRenderPipeline::~CRenderPipeline()
{
    Shutdown();
}

CRenderPipeline* CRenderPipeline::Instance()
{
    if (!m_pInstance)
        m_pInstance = new CRenderPipeline();
    return m_pInstance;
}

bool CRenderPipeline::Initialize()
{
    if (m_bInitialized)
        return true;

    if (!CreateGBuffer())
        return false;

    if (!CreateShaders())
        return false;

    m_bInitialized = true;
    return true;
}

void CRenderPipeline::Shutdown()
{
    if (!m_bInitialized)
        return;

    ReleaseGBuffer();
    m_bInitialized = false;
}

void CRenderPipeline::BeginDepthPass()
{
    if (!m_bInitialized)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, m_GBufferFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void CRenderPipeline::RenderGeometry(uint32_t drawCalls)
{
    m_DrawCallCount = drawCalls;
}

void CRenderPipeline::RenderLighting()
{
    if (m_bDeferredLighting)
    {
        glUseProgram(m_LightingShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Renderizar para screen
    }
}

void CRenderPipeline::RenderPostProcess()
{
    // Post-processing: SSAO, SSR, Motion Blur, etc
}

void CRenderPipeline::RenderScaleform()
{
    // Renderização de Scaleform UI
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
}

void CRenderPipeline::ComposeFinalImage()
{
    glUseProgram(m_CompositeShader);
    // Composição final com tonemap, color grading, etc
}

void CRenderPipeline::SetLightingMode(bool deferred)
{
    m_bDeferredLighting = deferred;
}

bool CRenderPipeline::CreateGBuffer()
{
    uint32_t width = 1280;
    uint32_t height = 720;

    glGenFramebuffers(1, &m_GBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_GBufferFBO);

    // Position texture
    glGenTextures(1, &m_PositionTexture);
    glBindTexture(GL_TEXTURE_2D, m_PositionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PositionTexture, 0);

    // Normal texture
    glGenTextures(1, &m_NormalTexture);
    glBindTexture(GL_TEXTURE_2D, m_NormalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_NormalTexture, 0);

    // Albedo texture
    glGenTextures(1, &m_AlbedoTexture);
    glBindTexture(GL_TEXTURE_2D, m_AlbedoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_AlbedoTexture, 0);

    // Depth texture
    glGenTextures(1, &m_DepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

bool CRenderPipeline::CreateShaders()
{
    // Criar shaders padrão
    // Esto vai ser preenchido com shaders reais
    return true;
}

void CRenderPipeline::ReleaseGBuffer()
{
    if (m_GBufferFBO != 0) glDeleteFramebuffers(1, &m_GBufferFBO);
    if (m_PositionTexture != 0) glDeleteTextures(1, &m_PositionTexture);
    if (m_NormalTexture != 0) glDeleteTextures(1, &m_NormalTexture);
    if (m_AlbedoTexture != 0) glDeleteTextures(1, &m_AlbedoTexture);
    if (m_DepthTexture != 0) glDeleteTextures(1, &m_DepthTexture);
}

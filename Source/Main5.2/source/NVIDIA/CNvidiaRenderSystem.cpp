#include "stdafx.h"
#include "CNvidiaRenderSystem.h"
#include "CNvidiaGPUManager.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

CNvidiaRenderSystem* CNvidiaRenderSystem::m_pInstance = nullptr;

CNvidiaRenderSystem::CNvidiaRenderSystem()
    : m_bInitialized(false)
    , m_GBuffer(0)
    , m_GBufferPositionTexture(0)
    , m_GBufferNormalTexture(0)
    , m_GBufferAlbedoTexture(0)
    , m_GBufferDepthTexture(0)
    , m_DeferredLightingShader(0)
    , m_PostProcessShader(0)
    , m_SSAOShader(0)
    , m_SSRShader(0)
{
    memset(&m_Config, 0, sizeof(m_Config));
    m_Config.Type = RENDER_TYPE_FORWARD;
    m_Config.MaxLights = 32;
    m_Config.ShadowResolution = 2048;
}

CNvidiaRenderSystem::~CNvidiaRenderSystem()
{
    Shutdown();
}

CNvidiaRenderSystem* CNvidiaRenderSystem::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CNvidiaRenderSystem();
    }
    return m_pInstance;
}

bool CNvidiaRenderSystem::Initialize(const RenderConfig& config)
{
    if (m_bInitialized)
        return true;

    m_Config = config;

    // Inicializar GPU Manager
    if (!gNvidiaGPU->IsNvidiaSupported())
    {
        std::cerr << "[NVIDIA Render] GPU NVIDIA não suportada!" << std::endl;
        return false;
    }

    // Compilar shaders se necessário
    if (!CompileShaders())
    {
        std::cerr << "[NVIDIA Render] Falha ao compilar shaders!" << std::endl;
        return false;
    }

    // Criar G-Buffer para renderização deferred
    if (m_Config.Type == RENDER_TYPE_DEFERRED)
    {
        if (!CreateGBuffer(1920, 1080))
        {
            std::cerr << "[NVIDIA Render] Falha ao criar G-Buffer!" << std::endl;
            return false;
        }
    }

    m_bInitialized = true;
    std::cout << "[NVIDIA Render] Sistema de renderização inicializado!" << std::endl;
    std::cout << "[NVIDIA Render] Tipo: ";
    switch (m_Config.Type)
    {
    case RENDER_TYPE_FORWARD:
        std::cout << "Forward Rendering" << std::endl;
        break;
    case RENDER_TYPE_DEFERRED:
        std::cout << "Deferred Rendering" << std::endl;
        break;
    case RENDER_TYPE_COMPUTE:
        std::cout << "Compute Shader Rendering" << std::endl;
        break;
    }

    return true;
}

void CNvidiaRenderSystem::Shutdown()
{
    if (!m_bInitialized)
        return;

    DeleteGBuffer();

    if (m_DeferredLightingShader != 0)
        glDeleteProgram(m_DeferredLightingShader);
    if (m_PostProcessShader != 0)
        glDeleteProgram(m_PostProcessShader);
    if (m_SSAOShader != 0)
        glDeleteProgram(m_SSAOShader);
    if (m_SSRShader != 0)
        glDeleteProgram(m_SSRShader);

    m_bInitialized = false;
    std::cout << "[NVIDIA Render] Sistema finalizado!" << std::endl;
}

void CNvidiaRenderSystem::SetRenderConfig(const RenderConfig& config)
{
    m_Config = config;
}

const CNvidiaRenderSystem::RenderConfig& CNvidiaRenderSystem::GetRenderConfig() const
{
    return m_Config;
}

bool CNvidiaRenderSystem::RenderMesh(GLuint VAO, GLuint IndexBuffer, GLuint ShaderProgram,
                                      const glm::mat4& ModelMatrix, int IndexCount)
{
    if (!m_bInitialized)
        return false;

    glUseProgram(ShaderProgram);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);

    // Passar matriz ao shader
    GLint modelLoc = glGetUniformLocation(ShaderProgram, "uModel");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glUseProgram(0);

    return true;
}

bool CNvidiaRenderSystem::RenderMeshInstanced(GLuint VAO, GLuint IndexBuffer, GLuint ShaderProgram,
                                               const glm::mat4* ModelMatrices, int InstanceCount, int IndexCount)
{
    if (!m_bInitialized || InstanceCount <= 0)
        return false;

    glUseProgram(ShaderProgram);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);

    // Renderizar com instancing
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, InstanceCount, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    return true;
}

bool CNvidiaRenderSystem::ApplyPostProcess()
{
    if (!m_bInitialized || m_PostProcessShader == 0)
        return false;

    glUseProgram(m_PostProcessShader);
    // Aplicar pós-processamento
    glUseProgram(0);

    return true;
}

bool CNvidiaRenderSystem::ApplySSAO(GLuint NormalTexture, GLuint DepthTexture)
{
    if (!m_bInitialized || !m_Config.EnableSSAO || m_SSAOShader == 0)
        return false;

    glUseProgram(m_SSAOShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, NormalTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, DepthTexture);
    
    // Renderizar quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glUseProgram(0);
    return true;
}

bool CNvidiaRenderSystem::ApplySSR(GLuint NormalTexture, GLuint DepthTexture, GLuint ReflectionTexture)
{
    if (!m_bInitialized || !m_Config.EnableSSR || m_SSRShader == 0)
        return false;

    glUseProgram(m_SSRShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, NormalTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, DepthTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ReflectionTexture);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
    return true;
}

bool CNvidiaRenderSystem::ApplyMotionBlur(GLuint VelocityTexture)
{
    if (!m_bInitialized || !m_Config.EnableMotionBlur)
        return false;

    return true;
}

bool CNvidiaRenderSystem::DispatchCompute(GLuint ComputeShader, GLuint GroupX, GLuint GroupY, GLuint GroupZ)
{
    if (ComputeShader == 0)
        return false;

    glUseProgram(ComputeShader);
    glDispatchCompute(GroupX, GroupY, GroupZ);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glUseProgram(0);

    return true;
}

void CNvidiaRenderSystem::WaitForGPU()
{
    glFinish();
}

void CNvidiaRenderSystem::FlushGPUCommands()
{
    glFlush();
}

void CNvidiaRenderSystem::BeginTimestamp(const char* markerName)
{
    if (glGetString(GL_EXTENSIONS))
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, markerName);
    }
}

void CNvidiaRenderSystem::EndTimestamp(const char* markerName)
{
    if (glGetString(GL_EXTENSIONS))
    {
        glPopDebugGroup();
    }
}

float CNvidiaRenderSystem::GetTimestamp(const char* markerName) const
{
    return 0.0f;
}

bool CNvidiaRenderSystem::CreateGBuffer(int width, int height)
{
    glGenFramebuffers(1, &m_GBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer);

    // Position attachment
    glGenTextures(1, &m_GBufferPositionTexture);
    glBindTexture(GL_TEXTURE_2D, m_GBufferPositionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GBufferPositionTexture, 0);

    // Normal attachment
    glGenTextures(1, &m_GBufferNormalTexture);
    glBindTexture(GL_TEXTURE_2D, m_GBufferNormalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBufferNormalTexture, 0);

    // Albedo attachment
    glGenTextures(1, &m_GBufferAlbedoTexture);
    glBindTexture(GL_TEXTURE_2D, m_GBufferAlbedoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBufferAlbedoTexture, 0);

    // Depth attachment
    glGenTextures(1, &m_GBufferDepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_GBufferDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBufferDepthTexture, 0);

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "[NVIDIA] G-Buffer incompleto!" << std::endl;
        DeleteGBuffer();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void CNvidiaRenderSystem::DeleteGBuffer()
{
    if (m_GBuffer != 0)
    {
        glDeleteFramebuffers(1, &m_GBuffer);
        m_GBuffer = 0;
    }

    if (m_GBufferPositionTexture != 0)
    {
        glDeleteTextures(1, &m_GBufferPositionTexture);
        m_GBufferPositionTexture = 0;
    }

    if (m_GBufferNormalTexture != 0)
    {
        glDeleteTextures(1, &m_GBufferNormalTexture);
        m_GBufferNormalTexture = 0;
    }

    if (m_GBufferAlbedoTexture != 0)
    {
        glDeleteTextures(1, &m_GBufferAlbedoTexture);
        m_GBufferAlbedoTexture = 0;
    }

    if (m_GBufferDepthTexture != 0)
    {
        glDeleteTextures(1, &m_GBufferDepthTexture);
        m_GBufferDepthTexture = 0;
    }
}

bool CNvidiaRenderSystem::CompileShaders()
{
    // Shaders serão compilados quando necessário
    // Por enquanto, apenas placeholder
    return true;
}

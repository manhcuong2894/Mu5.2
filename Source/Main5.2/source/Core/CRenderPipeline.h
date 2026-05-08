#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <map>

/**
 * @file CRenderPipeline.h
 * @brief Pipeline otimizado de renderização para múltiplas GPUs
 * @details Coordena renderização de Shaders, Scaleform e Efeitos
 */

enum class PipelineStage
{
    DEPTH_PREPASS = 0,
    GEOMETRY = 1,
    LIGHTING = 2,
    POSTPROCESS = 3,
    SCALEFORM = 4,
    COMPOSITION = 5
};

/**
 * @class CRenderPipeline
 * @brief Pipeline de renderização deferred com suporte a múltiplas GPUs
 */
class CRenderPipeline
{
public:
    static CRenderPipeline* Instance();

    // Inicialização
    bool Initialize();
    void Shutdown();

    // Estágios da pipeline
    void BeginDepthPass();
    void RenderGeometry(uint32_t drawCalls);
    void RenderLighting();
    void RenderPostProcess();
    void RenderScaleform();
    void ComposeFinalImage();

    // G-Buffer
    GLuint GetGBuffer() const { return m_GBufferFBO; }
    GLuint GetNormalTexture() const { return m_NormalTexture; }
    GLuint GetAlbedoTexture() const { return m_AlbedoTexture; }
    GLuint GetDepthTexture() const { return m_DepthTexture; }
    GLuint GetEmissiveTexture() const { return m_EmissiveTexture; }

    // Shading
    void SetLightingMode(bool deferred);
    bool IsUsingDeferredLighting() const { return m_bDeferredLighting; }

    // Performance
    uint32_t GetDrawCallCount() const { return m_DrawCallCount; }
    uint32_t GetTriangleCount() const { return m_TriangleCount; }

    virtual ~CRenderPipeline();

private:
    CRenderPipeline();
    static CRenderPipeline* m_pInstance;

    bool m_bInitialized;
    bool m_bDeferredLighting;

    // G-Buffer
    GLuint m_GBufferFBO;
    GLuint m_NormalTexture;
    GLuint m_AlbedoTexture;
    GLuint m_DepthTexture;
    GLuint m_EmissiveTexture;
    GLuint m_PositionTexture;

    // Shaders pipeline
    GLuint m_GeometryShader;
    GLuint m_LightingShader;
    GLuint m_CompositeShader;

    // Estatísticas
    uint32_t m_DrawCallCount;
    uint32_t m_TriangleCount;

    // Métodos privados
    bool CreateGBuffer();
    bool CreateShaders();
    void ReleaseGBuffer();
};

#define gRenderPipeline (CRenderPipeline::Instance())

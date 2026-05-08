#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

/**
 * @file CAMDRenderSystem.h
 * @brief Sistema de renderização otimizado para AMD
 * @details Renderização deferred compatível com placas AMD
 * 
 * Características:
 * - Renderização Deferred otimizada para AMD
 * - Suporte a múltiplas luzes
 * - Instancing de geometria
 * - Culling na GPU
 * - Shader assembly otimizado
 */

class CAMDRenderSystem
{
public:
    // Configuração de renderização
    struct RenderConfig
    {
        // Renderização
        bool EnableDeferred;            // Deferred rendering
        bool EnableInstancing;          // Geometry instancing
        
        // Otimizações AMD
        bool EnableWaveOptimization;    // Wave64 optimization
        bool EnableLDSUsage;            // Local data store
        bool EnablePredicates;          // Predicate execution
        
        // Qualidade
        float DetailLevel;              // 0.0-1.0
        int MaxLights;                  // Máximo de luzes
        int ShadowResolution;           // Resolução de sombras
        
        // Performance
        bool EnableGPUCulling;          // Culling na GPU
        bool EnableAsyncCompute;        // Compute assíncrono (limitado)
        
        // Debug
        bool ShowDebugInfo;             // Informações de debug
        bool WireframeMode;             // Modo wireframe
    };

    // Estrutura de estatísticas
    struct RenderStats
    {
        int DrawCalls;
        int TrianglesRendered;
        float FrameTimeMS;
        float GPUTimeMS;
        int LightsRendered;
    };

    static CAMDRenderSystem* Instance();

    // Inicialização
    bool Initialize(const RenderConfig& config = GetDefaultConfig());
    void Shutdown();

    // Configuração
    void SetRenderConfig(const RenderConfig& config) { m_Config = config; }
    const RenderConfig& GetRenderConfig() const { return m_Config; }
    static RenderConfig GetDefaultConfig();

    // Renderização
    void BeginFrame();
    void BeginDeferredPass();
    void RenderLightPass();
    void EndFrame();

    // Estatísticas
    const RenderStats& GetStats() const { return m_Stats; }
    void ResetStats();

    // Shaders
    GLuint GetDeferredShader() const { return m_DeferredShader; }
    GLuint GetLightShader() const { return m_LightShader; }

    // Debug
    void PrintStats();
    void RenderDebugOverlay();

    virtual ~CAMDRenderSystem();

private:
    CAMDRenderSystem();

    static CAMDRenderSystem* m_pInstance;

    // Configuração
    RenderConfig m_Config;

    // Shaders
    GLuint m_DeferredShader;
    GLuint m_LightShader;
    GLuint m_CompositeShader;

    // Framebuffers
    GLuint m_DeferredFBO;
    GLuint m_PositionTexture;
    GLuint m_NormalTexture;
    GLuint m_AlbedoTexture;
    GLuint m_DepthTexture;

    // Estatísticas
    RenderStats m_Stats;

    bool m_bInitialized;

    // Métodos privados
    bool CompileShaders();
    bool CreateFramebuffers();
    void CleanupFramebuffers();
};

#define gAMDRender (CAMDRenderSystem::Instance())

#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

/**
 * @file CIntelRenderSystem.h
 * @brief Sistema de renderização otimizado para Intel
 * @details Renderização forward compatível com GPUs Intel
 * 
 * Características:
 * - Renderização Forward otimizada
 * - Suporte a Light Culling
 * - Instancing de geometria
 * - Otimizações EU (Execution Units)
 * - Texture Filtering otimizado
 */

class CIntelRenderSystem
{
public:
    // Configuração de renderização
    struct RenderConfig
    {
        // Renderização
        bool EnableForward;             // Forward rendering
        bool EnableInstancing;          // Geometry instancing
        
        // Otimizações Intel
        bool EnableEUOptimization;      // EU (Execution Units) optimization
        bool EnablePredicates;          // Predicate execution
        bool EnableEarlyZ;              // Early Z rejection
        
        // Qualidade
        float DetailLevel;              // 0.0-1.0
        int MaxLightsPerObject;         // Luzes por objeto
        int ShadowResolution;           // Resolução de sombras
        
        // Performance
        bool EnableGPUCulling;          // Culling na GPU
        bool EnableLightCulling;        // Light culling
        
        // Debug
        bool ShowDebugInfo;             // Informações de debug
        bool WireframeMode;             // Modo wireframe
    };

    // Estrutura de estatísticas
    struct RenderStats
    {
        int DrawCalls;
        int TrianglesRendered;
        int LightsPerFrame;
        float FrameTimeMS;
        float GPUTimeMS;
    };

    static CIntelRenderSystem* Instance();

    // Inicialização
    bool Initialize(const RenderConfig& config = GetDefaultConfig());
    void Shutdown();

    // Configuração
    void SetRenderConfig(const RenderConfig& config) { m_Config = config; }
    const RenderConfig& GetRenderConfig() const { return m_Config; }
    static RenderConfig GetDefaultConfig();

    // Renderização
    void BeginFrame();
    void RenderObject(GLuint vao, GLuint shader, int indexCount);
    void EndFrame();

    // Estatísticas
    const RenderStats& GetStats() const { return m_Stats; }
    void ResetStats();

    // Shaders
    GLuint GetForwardShader() const { return m_ForwardShader; }

    // Debug
    void PrintStats();
    void RenderDebugOverlay();

    virtual ~CIntelRenderSystem();

private:
    CIntelRenderSystem();

    static CIntelRenderSystem* m_pInstance;

    // Configuração
    RenderConfig m_Config;

    // Shaders
    GLuint m_ForwardShader;
    GLuint m_LightCullingShader;

    // Buffers
    GLuint m_LightBuffer;
    GLuint m_LightCullingBuffer;

    // Estatísticas
    RenderStats m_Stats;

    bool m_bInitialized;

    // Métodos privados
    bool CompileShaders();
    void UpdateLightCulling();
};

#define gIntelRender (CIntelRenderSystem::Instance())

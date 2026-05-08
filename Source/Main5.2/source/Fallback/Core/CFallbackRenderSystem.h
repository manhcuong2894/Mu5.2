#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

/**
 * @file CFallbackRenderSystem.h
 * @brief Sistema de renderização fallback (OpenGL básico)
 * @details Renderização compatível com qualquer GPU
 * 
 * Características:
 * - Forward rendering puro
 * - Sem features avançadas
 * - Máxima compatibilidade
 * - Performance aceitável
 */

class CFallbackRenderSystem
{
public:
    // Configuração de renderização
    struct RenderConfig
    {
        bool EnableBasicLighting;       // Iluminação básica
        bool EnableShadows;             // Sombras básicas
        bool EnableTexturing;           // Texturas
        
        float DetailLevel;              // 0.0-1.0
        int MaxLights;                  // Máximo de luzes
        int MaxTextureSize;             // Tamanho máximo de textura
        
        bool ShowDebugInfo;             // Informações de debug
    };

    // Estrutura de estatísticas
    struct RenderStats
    {
        int DrawCalls;
        int TrianglesRendered;
        float FrameTimeMS;
    };

    static CFallbackRenderSystem* Instance();

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
    GLuint GetBasicShader() const { return m_BasicShader; }

    // Debug
    void PrintStats();

    virtual ~CFallbackRenderSystem();

private:
    CFallbackRenderSystem();

    static CFallbackRenderSystem* m_pInstance;

    // Configuração
    RenderConfig m_Config;

    // Shaders
    GLuint m_BasicShader;

    // Estatísticas
    RenderStats m_Stats;

    bool m_bInitialized;

    // Métodos privados
    bool CompileBasicShader();
};

#define gFallbackRender (CFallbackRenderSystem::Instance())

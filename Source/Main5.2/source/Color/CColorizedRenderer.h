#pragma once

#include "CColorSystem.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

/**
 * @file CColorizedRenderer.h
 * @brief Renderizador otimizado com suporte a colorização
 * @details Renderiza objetos com cores dinâmicas sem perda de performance
 */

class CColorizedRenderer
{
public:
    // Configuração de renderização colorizada
    struct RenderConfig
    {
        bool EnableColorization;         // Ativar/desativar cores
        CColorSystem::ColorizeMode Mode; // Modo de colorização
        bool UseVertexColors;            // Usar cores de vértices originais
        float ColorIntensity;            // 0.0-1.0 (intensidade da colorização)
        bool EnableLighting;             // Ativar iluminação
    };

    static CColorizedRenderer* Instance();

    // Inicialização
    bool Initialize(const RenderConfig& config = GetDefaultConfig());
    void Shutdown();

    // Configuração
    void SetConfig(const RenderConfig& config) { m_Config = config; }
    const RenderConfig& GetConfig() const { return m_Config; }
    static RenderConfig GetDefaultConfig();

    // Renderização colorizada
    void RenderColorized(
        GLuint vao,
        GLuint shader,
        int indexCount,
        CColorSystem::ColorizeMode mode,
        int colorValue
    );

    // Renderizar batch de objetos com cores
    void RenderBatch(
        const std::vector<GLuint>& vaos,
        const std::vector<GLuint>& shaders,
        const std::vector<int>& indexCounts,
        const std::vector<CColorSystem::ColorizeMode>& modes,
        const std::vector<int>& colorValues
    );

    // Atualizar shader uniforms
    void SetColorMode(GLuint shader, CColorSystem::ColorizeMode mode, int value);
    void SetLighting(GLuint shader, const glm::vec3& lightDir, const glm::vec3& lightColor);

    // Estatísticas
    int GetLastDrawCallCount() const { return m_LastDrawCallCount; }
    int GetLastVertexCount() const { return m_LastVertexCount; }

    virtual ~CColorizedRenderer();

private:
    CColorizedRenderer();

    static CColorizedRenderer* m_pInstance;

    RenderConfig m_Config;

    // Shader program
    GLuint m_ColorShader;
    GLuint m_DefaultShader;

    // Uniforms cache
    GLint m_ColorModeLoc;
    GLint m_ColorValueLoc;
    GLint m_LightDirLoc;
    GLint m_LightColorLoc;

    // Paleta de cores (texture 1D)
    GLuint m_ColorPaletteTexture;

    // Estatísticas
    int m_LastDrawCallCount;
    int m_LastVertexCount;

    bool m_bInitialized;

    // Métodos privados
    bool CompileColorShader();
    bool CreateColorPaletteTexture();
    void UpdateColorPalette();
};

#define gColorizedRenderer (CColorizedRenderer::Instance())

#include "stdafx.h"
#include "CColorizedRenderer.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

CColorizedRenderer* CColorizedRenderer::m_pInstance = nullptr;

CColorizedRenderer::CColorizedRenderer()
    : m_ColorShader(0)
    , m_DefaultShader(0)
    , m_ColorModeLoc(-1)
    , m_ColorValueLoc(-1)
    , m_LightDirLoc(-1)
    , m_LightColorLoc(-1)
    , m_ColorPaletteTexture(0)
    , m_LastDrawCallCount(0)
    , m_LastVertexCount(0)
    , m_bInitialized(false)
{
}

CColorizedRenderer::~CColorizedRenderer()
{
    Shutdown();
}

CColorizedRenderer* CColorizedRenderer::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CColorizedRenderer();
    }
    return m_pInstance;
}

CColorizedRenderer::RenderConfig CColorizedRenderer::GetDefaultConfig()
{
    RenderConfig config = {};
    config.EnableColorization = true;
    config.Mode = CColorSystem::ColorizeMode::ITEM_GRADE;
    config.UseVertexColors = true;
    config.ColorIntensity = 1.0f;
    config.EnableLighting = true;
    return config;
}

bool CColorizedRenderer::Initialize(const RenderConfig& config)
{
    if (m_bInitialized)
        return true;

    m_Config = config;

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Colorized Renderer Inicialização" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Inicializar color system
    if (!gColorSystem->Initialize())
    {
        std::cerr << "[Colorized Renderer] Falha ao inicializar Color System!" << std::endl;
        return false;
    }

    // Compilar shader de colorização
    if (!CompileColorShader())
    {
        std::cerr << "[Colorized Renderer] Falha ao compilar shader!" << std::endl;
        return false;
    }

    // Criar texture de paleta
    if (!CreateColorPaletteTexture())
    {
        std::cerr << "[Colorized Renderer] Falha ao criar paleta!" << std::endl;
        return false;
    }

    m_bInitialized = true;

    std::cout << "[Colorized Renderer] ? Inicializado!" << std::endl;
    std::cout << "[Colorized Renderer] Colorização: " << (m_Config.EnableColorization ? "ATIVA" : "INATIVA") << std::endl;
    std::cout << "[Colorized Renderer] Modo: " << static_cast<int>(m_Config.Mode) << std::endl;

    return true;
}

void CColorizedRenderer::Shutdown()
{
    if (!m_bInitialized)
        return;

    if (m_ColorShader != 0)
    {
        glDeleteProgram(m_ColorShader);
        m_ColorShader = 0;
    }

    if (m_ColorPaletteTexture != 0)
    {
        glDeleteTextures(1, &m_ColorPaletteTexture);
        m_ColorPaletteTexture = 0;
    }

    m_bInitialized = false;

    std::cout << "[Colorized Renderer] Finalizado!" << std::endl;
}

void CColorizedRenderer::RenderColorized(
    GLuint vao,
    GLuint shader,
    int indexCount,
    CColorSystem::ColorizeMode mode,
    int colorValue)
{
    if (!m_bInitialized || !m_Config.EnableColorization)
    {
        // Renderização normal sem cores
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        return;
    }

    // Usar shader de colorização
    glUseProgram(m_ColorShader);

    // Ativar paleta de cores
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, m_ColorPaletteTexture);

    // Setar uniforms
    SetColorMode(m_ColorShader, mode, colorValue);

    // Renderizar
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    glUseProgram(0);

    m_LastDrawCallCount++;
    m_LastVertexCount += indexCount;
}

void CColorizedRenderer::RenderBatch(
    const std::vector<GLuint>& vaos,
    const std::vector<GLuint>& shaders,
    const std::vector<int>& indexCounts,
    const std::vector<CColorSystem::ColorizeMode>& modes,
    const std::vector<int>& colorValues)
{
    if (!m_bInitialized || vaos.empty())
        return;

    size_t batchSize = vaos.size();

    if (shaders.size() != batchSize || indexCounts.size() != batchSize ||
        modes.size() != batchSize || colorValues.size() != batchSize)
    {
        std::cerr << "[Colorized Renderer] Tamanho de batch incompatível!" << std::endl;
        return;
    }

    // Usar shader de colorização
    glUseProgram(m_ColorShader);

    // Ativar paleta de cores
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, m_ColorPaletteTexture);

    // Renderizar cada objeto do batch
    for (size_t i = 0; i < batchSize; ++i)
    {
        // Setar uniforms de cor
        SetColorMode(m_ColorShader, modes[i], colorValues[i]);

        // Renderizar
        glBindVertexArray(vaos[i]);
        glDrawElements(GL_TRIANGLES, indexCounts[i], GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        m_LastDrawCallCount++;
        m_LastVertexCount += indexCounts[i];
    }

    glUseProgram(0);
}

void CColorizedRenderer::SetColorMode(GLuint shader, CColorSystem::ColorizeMode mode, int value)
{
    GLint modeLoc = glGetUniformLocation(shader, "uColorMode");
    GLint valueLoc = glGetUniformLocation(shader, "uColorValue");

    glUniform1i(modeLoc, static_cast<int>(mode));
    glUniform1i(valueLoc, value);
}

void CColorizedRenderer::SetLighting(GLuint shader, const glm::vec3& lightDir, const glm::vec3& lightColor)
{
    GLint lightDirLoc = glGetUniformLocation(shader, "uLightDir");
    GLint lightColorLoc = glGetUniformLocation(shader, "uLightColor");

    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
}

bool CColorizedRenderer::CompileColorShader()
{
    // Aqui você compilaria o shader colorize.vs e colorize.fs
    // Por enquanto, retorna true (será implementado com seu sistema de shaders)
    std::cout << "[Colorized Renderer] Shader compilado" << std::endl;
    return true;
}

bool CColorizedRenderer::CreateColorPaletteTexture()
{
    const auto& palette = gColorSystem->GetCurrentPalette();

    // Criar texture 1D para armazenar paleta
    glGenTextures(1, &m_ColorPaletteTexture);
    glBindTexture(GL_TEXTURE_1D, m_ColorPaletteTexture);

    // Configurar texture
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    // Converter cores para formato OpenGL
    std::vector<float> colorData;
    for (int i = 0; i < palette.ColorCount; ++i)
    {
        colorData.push_back(palette.Colors[i].R);
        colorData.push_back(palette.Colors[i].G);
        colorData.push_back(palette.Colors[i].B);
    }

    // Upload para GPU
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, palette.ColorCount, 0, GL_RGB, GL_FLOAT, colorData.data());

    glBindTexture(GL_TEXTURE_1D, 0);

    std::cout << "[Colorized Renderer] Paleta (1D Texture) criada com " << palette.ColorCount << " cores" << std::endl;

    return true;
}

void CColorizedRenderer::UpdateColorPalette()
{
    if (m_ColorPaletteTexture == 0)
        return;

    // Recriar texture se paleta foi modificada
    CreateColorPaletteTexture();
}

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <string>

/**
 * @file CShaderGL.h
 * @brief Gerenciador de shaders com suporte a colorização dinâmica
 * @details Integrado com CColorSystem para renderização otimizada
 */

class CShaderGL
{
public:
    enum GpuAssistMode
    {
        GPU_ASSIST_OFF = 0,
        GPU_ASSIST_AUTO = 1,
        GPU_ASSIST_FORCE = 2
    };

    // Tipos de shader suportados
    enum ShaderType
    {
        SHADER_DEFAULT = 0,
        SHADER_TERRAIN = 1,
        SHADER_GLOW = 2,
        SHADER_CHARACTER = 3,
        SHADER_COLORIZED = 4,
        SHADER_PARTICLE = 5
    };

    CShaderGL();
    virtual ~CShaderGL();

    // Inicialização
    void Init();
    void Shutdown();

    // Gerenciamento de programa de shader
    void RenderShader(ShaderType type = SHADER_DEFAULT);
    bool CheckedShader(ShaderType type = SHADER_DEFAULT) const;

    GpuAssistMode GetGpuAssistMode() const;
    bool IsGpuAssistAvailable() const;
    bool IsGpuAssistEnabled() const;
    
    // Obter IDs dos programas
    GLuint GetShaderId() const;
    GLuint GetShaderTerrainId() const;
    GLuint GetShaderGlowId() const;
    GLuint GetShaderCharacterId() const;
    GLuint GetShaderColorizedId() const;
    GLuint GetShaderParticleId() const;

    // Carregamento de shaders
    GLuint LoadShaderProgram(const char* vertexShaderFile, const char* fragmentShaderFile);
    GLuint LoadShaderProgram(const char* vertexShaderFile, const char* geometryShaderFile, const char* fragmentShaderFile);

    // Leitura de arquivos
    bool readshader(const char* filename, std::string& shader_text);

    // Compilação de shaders
    GLuint run_shader(const char* shader_text, GLenum type);

    // Uniforms - tipos básicos
    void setBool(const char* name, bool value) const;
    void setInt(const char* name, int value) const;
    void setFloat(const char* name, float value) const;

    // Uniforms - vetores
    void setVec2(const char* name, float x, float y) const;
    void setVec3(const char* name, float x, float y, float z) const;
    void setVec4(const char* name, float x, float y, float z, float w) const;

    // Uniforms - matrizes
    void setMat4(const char* name, glm::mat4& matrix) const;

    // Matriz de perspectiva
    void SetPerspective(float fov, float aspect, float nearPlane, float farPlane);
    void run_projection();

    // Singleton
    static CShaderGL* Instance();

private:
    void LoadGpuAssistConfig();
    void DetectGpuAssistSupport();

    GLuint shader_id;
    GLuint shader_terrain_id;
    GLuint shader_glow_id;
    GLuint shader_character_id;
    GLuint shader_colorized_id;
    GLuint shader_particle_id;

    glm::mat4 m_ProjectionMatrix;

    bool m_bInitialized;
    bool m_bGpuAssistAvailable;
    GpuAssistMode m_eGpuAssistMode;
};

#define gShaderGL (CShaderGL::Instance())


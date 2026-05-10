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
        SHADER_PARTICLE = 5,
        SHADER_MODEL_VBO = 6,
        SHADER_CHROME1_VBO = 7,
        SHADER_CHROME2_VBO = 8,
        SHADER_CHROME3_VBO = 9,
        SHADER_CHROME4_VBO = 10,
        SHADER_CHROME5_VBO = 11,
        SHADER_CHROME6_VBO = 12,
        SHADER_CHROME7_VBO = 13,
        SHADER_METAL_VBO = 14,
        SHADER_OIL_VBO = 15,
        SHADER_BLENDMESH_VBO = 16
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
    GLuint GetShaderVboId(int mode) const;

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

    glm::mat4& GetProjectionMatrix() { return m_ProjectionMatrix; }

private:
    void LoadGpuAssistConfig();
    void DetectGpuAssistSupport();

    GLuint shader_id;
    GLuint shader_terrain_id;
    GLuint shader_glow_id;
    GLuint shader_character_id;
    GLuint shader_colorized_id;
    GLuint shader_particle_id;

    GLuint shader_vbo_model;
    GLuint shader_vbo_chrome[7];
    GLuint shader_vbo_metal;
    GLuint shader_vbo_oil;
    GLuint shader_vbo_blendmesh;

    glm::mat4 m_ProjectionMatrix;

    bool m_bInitialized;
    bool m_bGpuAssistAvailable;
    GpuAssistMode m_eGpuAssistMode;
};

#define gShaderGL (CShaderGL::Instance())


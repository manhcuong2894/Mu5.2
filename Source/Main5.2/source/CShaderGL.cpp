#include "stdafx.h"
#include "CShaderGL.h"

#ifdef SHADER_VERSION_TEST
#include "Utilities/Log/muConsoleDebug.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

CShaderGL::CShaderGL()
    : shader_id(0),
      shader_terrain_id(0),
      shader_glow_id(0),
      shader_character_id(0),
      shader_colorized_id(0),
      shader_particle_id(0),
      shader_vbo_model(0),
      shader_vbo_metal(0),
      shader_vbo_oil(0),
      shader_vbo_blendmesh(0),
      m_ProjectionMatrix(glm::mat4(1.0f)),
      m_bInitialized(false),
      m_bGpuAssistAvailable(false),
      m_eGpuAssistMode(GPU_ASSIST_AUTO)
{
    for (int i = 0; i < 7; ++i) shader_vbo_chrome[i] = 0;
}

CShaderGL::~CShaderGL()
{
    Shutdown();
}

CShaderGL* CShaderGL::Instance()
{
    static CShaderGL sInstance;
    return &sInstance;
}

void CShaderGL::Init()
{
    if (m_bInitialized)
        return;

    this->shader_id =
        this->LoadShaderProgram("Shaders/shader.vs", "Shaders/shader.fs");
    this->shader_terrain_id =
        this->LoadShaderProgram("Shaders/terrain.vs", "Shaders/terrain.fs");
    this->shader_glow_id =
        this->LoadShaderProgram("Shaders/glow.vs", "Shaders/glow.fs");
    this->shader_character_id =
        this->LoadShaderProgram("Shaders/character.vs", "Shaders/character.fs");
    this->shader_colorized_id =
        this->LoadShaderProgram("Shaders/colorize.vs", "Shaders/colorize.fs");
    this->shader_particle_id =
        this->LoadShaderProgram("Shaders/particle.vs", "Shaders/particle.gs", "Shaders/particle.fs");

    // Load VBO Specialized Shaders
    this->shader_vbo_model = this->LoadShaderProgram("VBO/Model.vs", "VBO/Model.fs");
    this->shader_vbo_chrome[0] = this->LoadShaderProgram("VBO/Chrome1.vs", "VBO/Chrome1.fs");
    this->shader_vbo_chrome[1] = this->LoadShaderProgram("VBO/Chrome2.vs", "VBO/Chrome2.fs");
    this->shader_vbo_chrome[2] = this->LoadShaderProgram("VBO/Chrome3.vs", "VBO/Chrome3.fs");
    this->shader_vbo_chrome[3] = this->LoadShaderProgram("VBO/Chrome4.vs", "VBO/Chrome4.fs");
    this->shader_vbo_chrome[4] = this->LoadShaderProgram("VBO/Chrome5.vs", "VBO/Chrome5.fs");
    this->shader_vbo_chrome[5] = this->LoadShaderProgram("VBO/Chrome6.vs", "VBO/Chrome6.fs");
    this->shader_vbo_chrome[6] = this->LoadShaderProgram("VBO/Chrome7.vs", "VBO/Chrome7.fs");
    this->shader_vbo_metal = this->LoadShaderProgram("VBO/Metal.vs", "VBO/Metal.fs");
    this->shader_vbo_oil = this->LoadShaderProgram("VBO/Oil.vs", "VBO/Oil.fs");
    this->shader_vbo_blendmesh = this->LoadShaderProgram("VBO/BlendMesh.vs", "VBO/BlendMesh.fs");

    LoadGpuAssistConfig();
    DetectGpuAssistSupport();

    m_bInitialized = (shader_id != 0);

    if (!m_bInitialized)
    {
        g_ConsoleDebug->Write(MCD_ERROR, "[CShaderGL] Falha ao inicializar shaders");
    }
}

void CShaderGL::Shutdown()
{
    if (shader_id != 0) glDeleteProgram(shader_id);
    if (shader_terrain_id != 0) glDeleteProgram(shader_terrain_id);
    if (shader_glow_id != 0) glDeleteProgram(shader_glow_id);
    if (shader_character_id != 0) glDeleteProgram(shader_character_id);
    if (shader_colorized_id != 0) glDeleteProgram(shader_colorized_id);
    if (shader_particle_id != 0) glDeleteProgram(shader_particle_id);
    if (shader_vbo_model != 0) glDeleteProgram(shader_vbo_model);
    for (int i = 0; i < 7; ++i) if (shader_vbo_chrome[i] != 0) glDeleteProgram(shader_vbo_chrome[i]);
    if (shader_vbo_metal != 0) glDeleteProgram(shader_vbo_metal);
    if (shader_vbo_oil != 0) glDeleteProgram(shader_vbo_oil);
    if (shader_vbo_blendmesh != 0) glDeleteProgram(shader_vbo_blendmesh);

    shader_id = 0;
    shader_terrain_id = 0;
    shader_glow_id = 0;
    shader_character_id = 0;
    shader_colorized_id = 0;
    shader_particle_id = 0;
    m_bInitialized = false;
    m_bGpuAssistAvailable = false;
}

void CShaderGL::RenderShader(ShaderType type)
{
    GLuint shader = 0;

    switch (type)
    {
        case SHADER_TERRAIN:
            shader = shader_terrain_id;
            break;
        case SHADER_GLOW:
            shader = shader_glow_id;
            break;
        case SHADER_CHARACTER:
            shader = shader_character_id;
            break;
        case SHADER_COLORIZED:
            shader = shader_colorized_id;
            break;
        case SHADER_PARTICLE:
            shader = shader_particle_id;
            break;
        default:
            shader = shader_id;
            break;
    }

    if (shader != 0)
    {
        glUseProgram(shader);
    }
}

bool CShaderGL::CheckedShader(ShaderType type) const
{
    switch (type)
    {
        case SHADER_TERRAIN:
            return shader_terrain_id != 0;
        case SHADER_GLOW:
            return shader_glow_id != 0;
        case SHADER_CHARACTER:
            return shader_character_id != 0;
        case SHADER_COLORIZED:
            return shader_colorized_id != 0;
        case SHADER_PARTICLE:
            return shader_particle_id != 0;
        default:
            return shader_id != 0;
    }
}

CShaderGL::GpuAssistMode CShaderGL::GetGpuAssistMode() const
{
    return m_eGpuAssistMode;
}

bool CShaderGL::IsGpuAssistAvailable() const
{
    return m_bGpuAssistAvailable;
}

bool CShaderGL::IsGpuAssistEnabled() const
{
    if (m_eGpuAssistMode == GPU_ASSIST_OFF)
        return false;

    return m_bGpuAssistAvailable;
}

GLuint CShaderGL::GetShaderId() const
{
    return shader_id;
}

GLuint CShaderGL::GetShaderTerrainId() const
{
    return shader_terrain_id;
}

GLuint CShaderGL::GetShaderGlowId() const
{
    return shader_glow_id;
}

GLuint CShaderGL::GetShaderCharacterId() const
{
    return shader_character_id;
}

GLuint CShaderGL::GetShaderColorizedId() const
{
    return shader_colorized_id;
}

GLuint CShaderGL::GetShaderParticleId() const
{
    return shader_particle_id;
}

GLuint CShaderGL::GetShaderVboId(int mode) const
{
    switch (mode)
    {
    case 1: return shader_vbo_chrome[0];
    case 2: return shader_vbo_chrome[1];
    case 3: return shader_vbo_chrome[2];
    case 4: return shader_vbo_chrome[3];
    case 5: return shader_vbo_chrome[4];
    case 6: return shader_vbo_chrome[5];
    case 7: return shader_vbo_chrome[6];
    case 8: return shader_vbo_chrome[3]; // Chrome 8 often maps to Chrome 4 logic
    case 9: return shader_vbo_metal;
    case 10: return shader_vbo_oil;
    case 11: return shader_vbo_blendmesh;
    default: return shader_vbo_model;
    }
}

GLuint CShaderGL::LoadShaderProgram(const char* vertexShaderFile,
                                    const char* fragmentShaderFile)
{
    std::string vertexShaderSource, fragmentShaderSource;

    if (!this->readshader(vertexShaderFile, vertexShaderSource) ||
        !this->readshader(fragmentShaderFile, fragmentShaderSource))
        return 0;

    GLuint vertexShader =
        this->run_shader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
    GLuint fragmentShader =
        this->run_shader(fragmentShaderSource.c_str(), GL_FRAGMENT_SHADER);

    if (vertexShader == 0 || fragmentShader == 0)
        return 0;

    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShader);
    glAttachShader(programId, fragmentShader);
    glLinkProgram(programId);

    GLint success;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(programId, 512, NULL, infoLog);
        g_ConsoleDebug->Write(MCD_ERROR, "Shader Program Link Error (%s/%s): %s",
                              vertexShaderFile, fragmentShaderFile, infoLog);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return programId;
}

GLuint CShaderGL::LoadShaderProgram(const char* vertexShaderFile,
                                    const char* geometryShaderFile,
                                    const char* fragmentShaderFile)
{
    std::string vertexShaderSource, geometryShaderSource, fragmentShaderSource;

    if (!this->readshader(vertexShaderFile, vertexShaderSource) ||
        !this->readshader(geometryShaderFile, geometryShaderSource) ||
        !this->readshader(fragmentShaderFile, fragmentShaderSource))
        return 0;

    GLuint vertexShader =
        this->run_shader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
    GLuint geometryShader =
        this->run_shader(geometryShaderSource.c_str(), GL_GEOMETRY_SHADER);
    GLuint fragmentShader =
        this->run_shader(fragmentShaderSource.c_str(), GL_FRAGMENT_SHADER);

    if (vertexShader == 0 || geometryShader == 0 || fragmentShader == 0)
        return 0;

    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShader);
    glAttachShader(programId, geometryShader);
    glAttachShader(programId, fragmentShader);
    glLinkProgram(programId);

    GLint success;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(programId, 512, NULL, infoLog);
        g_ConsoleDebug->Write(MCD_ERROR, "Shader Program Link Error (%s/%s/%s): %s",
                              vertexShaderFile, geometryShaderFile, fragmentShaderFile, infoLog);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);
    return programId;
}

void CShaderGL::LoadGpuAssistConfig()
{
    char configValue[32] = {0};
    GetPrivateProfileStringA("Graphics", "UseGpuAssist", "Auto", configValue,
                             sizeof(configValue), ".\\config.ini");

    std::string mode = configValue;
    std::transform(mode.begin(), mode.end(), mode.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });

    if (mode == "0" || mode == "off" || mode == "false")
    {
        m_eGpuAssistMode = GPU_ASSIST_OFF;
    }
    else if (mode == "2" || mode == "force")
    {
        m_eGpuAssistMode = GPU_ASSIST_FORCE;
    }
    else
    {
        m_eGpuAssistMode = GPU_ASSIST_AUTO;
    }
}

void CShaderGL::DetectGpuAssistSupport()
{
    m_bGpuAssistAvailable = false;

    if (shader_character_id == 0 || !GLEW_VERSION_3_3)
        return;

    const char* vendorRaw = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* rendererRaw = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

    if (vendorRaw == NULL || rendererRaw == NULL)
        return;

    std::string vendor = vendorRaw;
    std::string renderer = rendererRaw;
    std::transform(vendor.begin(), vendor.end(), vendor.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    std::transform(renderer.begin(), renderer.end(), renderer.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });

    if (m_eGpuAssistMode != GPU_ASSIST_FORCE)
    {
        if (vendor.find("microsoft") != std::string::npos ||
            renderer.find("gdi generic") != std::string::npos ||
            renderer.find("software") != std::string::npos)
        {
            return;
        }
    }

    m_bGpuAssistAvailable = true;
}

bool CShaderGL::readshader(const char* filename, std::string& shader_text)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        // Try adding Data/ prefix if missing, common in MuOnline
        std::string altPath = "Data/";
        altPath += filename;
        file.open(altPath);
        if (!file.is_open())
        {
            g_ConsoleDebug->Write(MCD_ERROR, "Failed to open shader file: %s",
                                  filename);
            return false;
        }
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    shader_text = buffer.str();
    file.close();
    return true;
}

GLuint CShaderGL::run_shader(const char* shader_text, GLenum type)
{
    GLuint shaderId = glCreateShader(type);
    glShaderSource(shaderId, 1, &shader_text, NULL);
    glCompileShader(shaderId);

    GLint success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
        g_ConsoleDebug->Write(MCD_ERROR, "Shader Compilation Error: %s", infoLog);
        return 0;
    }
    return shaderId;
}

static glm::mat4 g_ProjectionMatrix = glm::mat4(1.0f);

void CShaderGL::run_projection()
{
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (program > 0)
    {
        GLint loc = glGetUniformLocation(program, "projection");
        if (loc != -1)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(g_ProjectionMatrix));
        }
    }
}

void CShaderGL::SetPerspective(float Fov, float Aspect, float ZNear,
                               float ZFar)
{
    g_ProjectionMatrix = glm::perspective(glm::radians(Fov), Aspect, ZNear, ZFar);
    m_ProjectionMatrix = g_ProjectionMatrix;
    this->run_projection();
}

void CShaderGL::setBool(const char* name, bool value) const
{
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (program > 0)
        glUniform1i(glGetUniformLocation(program, name), (int)value);
}

void CShaderGL::setInt(const char* name, int value) const
{
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (program > 0)
        glUniform1i(glGetUniformLocation(program, name), value);
}

void CShaderGL::setFloat(const char* name, float value) const
{
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (program > 0)
        glUniform1f(glGetUniformLocation(program, name), value);
}

void CShaderGL::setVec2(const char* name, float x, float y) const
{
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (program > 0)
        glUniform2f(glGetUniformLocation(program, name), x, y);
}

void CShaderGL::setVec3(const char* name, float x, float y, float z) const
{
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (program > 0)
        glUniform3f(glGetUniformLocation(program, name), x, y, z);
}

void CShaderGL::setVec4(const char* name, float x, float y, float z,
                        float w) const
{
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (program > 0)
        glUniform4f(glGetUniformLocation(program, name), x, y, z, w);
}

void CShaderGL::setMat4(const char* name, glm::mat4& matrix) const
{
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (program > 0)
        glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE,
                           glm::value_ptr(matrix));
}

#endif // SHADER_VERSION_TEST


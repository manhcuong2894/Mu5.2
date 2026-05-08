#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>

/**
 * @brief Gerenciador de renderização com suporte NVIDIA
 * @details Implementa renderização paralela e otimizada via GPU
 */
class CNvidiaRenderSystem
{
public:
    // Tipos de renderização
    enum RenderType
    {
        RENDER_TYPE_FORWARD = 0,      // Renderização forward tradicional
        RENDER_TYPE_DEFERRED = 1,     // Renderização deferred (G-Buffer)
        RENDER_TYPE_COMPUTE = 2,      // Renderização via compute shaders
    };

    // Configurações de renderização
    struct RenderConfig
    {
        RenderType Type;
        bool EnableSSAO;              // Screen Space Ambient Occlusion
        bool EnableSSR;               // Screen Space Reflection
        bool EnableMotionBlur;
        bool EnableTAA;               // Temporal Anti-Aliasing
        bool EnableAsyncCompute;      // Computação assíncrona
        int MaxLights;
        int ShadowResolution;
    };

    static CNvidiaRenderSystem* Instance();

    // Inicialização
    bool Initialize(const RenderConfig& config);
    void Shutdown();

    // Configuração
    void SetRenderConfig(const RenderConfig& config);
    const RenderConfig& GetRenderConfig() const;

    // Renderização de mesh
    bool RenderMesh(GLuint VAO, GLuint IndexBuffer, GLuint ShaderProgram, 
                    const glm::mat4& ModelMatrix, int IndexCount);

    // Renderização com instancing
    bool RenderMeshInstanced(GLuint VAO, GLuint IndexBuffer, GLuint ShaderProgram,
                             const glm::mat4* ModelMatrices, int InstanceCount, int IndexCount);

    // Post-processing
    bool ApplyPostProcess();
    bool ApplySSAO(GLuint NormalTexture, GLuint DepthTexture);
    bool ApplySSR(GLuint NormalTexture, GLuint DepthTexture, GLuint ReflectionTexture);
    bool ApplyMotionBlur(GLuint VelocityTexture);

    // Compute shaders
    bool DispatchCompute(GLuint ComputeShader, GLuint GroupX, GLuint GroupY, GLuint GroupZ);

    // Sincronização
    void WaitForGPU();
    void FlushGPUCommands();

    // Profiling
    void BeginTimestamp(const char* markerName);
    void EndTimestamp(const char* markerName);
    float GetTimestamp(const char* markerName) const;

    virtual ~CNvidiaRenderSystem();

private:
    CNvidiaRenderSystem();
    
    static CNvidiaRenderSystem* m_pInstance;
    
    RenderConfig m_Config;
    bool m_bInitialized;
    
    // G-Buffers para deferred rendering
    GLuint m_GBuffer;
    GLuint m_GBufferPositionTexture;
    GLuint m_GBufferNormalTexture;
    GLuint m_GBufferAlbedoTexture;
    GLuint m_GBufferDepthTexture;
    
    // Shaders
    GLuint m_DeferredLightingShader;
    GLuint m_PostProcessShader;
    GLuint m_SSAOShader;
    GLuint m_SSRShader;
    
    // Query objects para timing
    std::vector<GLuint> m_TimestampQueries;
    
    bool CreateGBuffer(int width, int height);
    void DeleteGBuffer();
    bool CompileShaders();
};

#define gNvidiaRender (CNvidiaRenderSystem::Instance())

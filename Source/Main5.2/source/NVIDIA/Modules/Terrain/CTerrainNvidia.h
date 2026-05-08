#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "TerrainNvidiaConfig.h"

/**
 * @file CTerrainNvidia.h
 * @brief Renderização NVIDIA otimizada para Terrain
 * @details Sistema modular para renderizar terreno com aceleração GPU
 * 
 * Uso:
 * @code
 * CTerrainNvidia terrain;
 * terrain.Initialize(TerrainNvidia::DEFAULT_CONFIG);
 * 
 * // Em cada frame
 * terrain.Update(cameraPosition);
 * terrain.Render(viewMatrix, projMatrix);
 * @endcode
 */
class CTerrainNvidia
{
public:
    // Estrutura para chunk de terreno
    struct TerrainChunk
    {
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
        GLuint TextureArray;
        
        glm::vec3 Position;
        float Size;
        int LODLevel;
        
        int VertexCount;
        int IndexCount;
        
        bool IsVisible;
    };

    // Estrutura para material
    struct TerrainMaterial
    {
        GLuint AlbedoTexture;
        GLuint NormalTexture;
        GLuint RoughnessTexture;
        GLuint MetallicTexture;
        GLuint AOTexture;
        
        glm::vec4 TintColor;
        float Roughness;
        float Metallic;
    };

    // Estrutura de estatísticas
    struct TerrainStats
    {
        int VisibleChunks;
        int TotalChunks;
        int VerticesRendered;
        int DrawCalls;
        float GPUTimeMS;
        float UpdateTimeMS;
    };

    static CTerrainNvidia* Instance();

    // Inicialização
    bool Initialize(const TerrainNvidia::Config& config = TerrainNvidia::DEFAULT_CONFIG);
    void Shutdown();

    // Configuração
    void SetConfig(const TerrainNvidia::Config& config) { m_Config = config; }
    const TerrainNvidia::Config& GetConfig() const { return m_Config; }

    // Gerenciamento de chunks
    bool AddChunk(int x, int z, float size, GLuint terrainData);
    bool RemoveChunk(int x, int z);
    void ClearAllChunks();

    // Materiais
    bool SetMaterial(int index, const TerrainMaterial& material);
    const TerrainMaterial& GetMaterial(int index) const;

    // Renderização
    void Update(const glm::vec3& cameraPosition);
    void Render(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);

    // LOD Management
    void SetLODDistance(float distance) { m_Config.LodDistance = distance; }
    int GetLODLevel(const glm::vec3& chunkPos, const glm::vec3& cameraPos) const;

    // Culling
    void UpdateFrustumCulling(const glm::mat4& viewProjMatrix);
    bool IsChunkVisible(const TerrainChunk& chunk) const;

    // Estatísticas
    const TerrainStats& GetStats() const { return m_Stats; }
    void ResetStats();

    // Debug
    void RenderDebugInfo();
    void SetDebugMode(bool enable) { m_bDebugMode = enable; }

    virtual ~CTerrainNvidia();

private:
    CTerrainNvidia();

    static CTerrainNvidia* m_pInstance;

    // Configuração
    TerrainNvidia::Config m_Config;
    
    // Chunks
    std::vector<TerrainChunk> m_Chunks;
    
    // Materiais
    std::vector<TerrainMaterial> m_Materials;
    
    // Shaders
    GLuint m_ShaderProgram;
    GLuint m_ComputeShader;
    
    // Statísticas
    TerrainStats m_Stats;
    
    // Estado
    bool m_bInitialized;
    bool m_bDebugMode;
    
    // Métodos privados
    bool CompileShaders();
    void UpdateChunkLOD();
    void RenderChunk(const TerrainChunk& chunk, const glm::mat4& viewMatrix, 
                     const glm::mat4& projMatrix);
    void UpdateComputeShader();
};

#define gTerrainNvidia (CTerrainNvidia::Instance())

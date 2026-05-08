#include "stdafx.h"
#include "CTerrainNvidia.h"
#include "../../CNvidiaSystem.h"
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

CTerrainNvidia* CTerrainNvidia::m_pInstance = nullptr;

CTerrainNvidia::CTerrainNvidia()
    : m_ShaderProgram(0)
    , m_ComputeShader(0)
    , m_bInitialized(false)
    , m_bDebugMode(false)
{
    memset(&m_Stats, 0, sizeof(m_Stats));
}

CTerrainNvidia::~CTerrainNvidia()
{
    Shutdown();
}

CTerrainNvidia* CTerrainNvidia::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CTerrainNvidia();
    }
    return m_pInstance;
}

bool CTerrainNvidia::Initialize(const TerrainNvidia::Config& config)
{
    if (m_bInitialized)
        return true;

    m_Config = config;

    // Verificar se NVIDIA está disponível
    if (!gNvidiaSystem->IsInitialized())
    {
        std::cerr << "[Terrain NVIDIA] Sistema NVIDIA não inicializado!" << std::endl;
        return false;
    }

    // Compilar shaders
    if (!CompileShaders())
    {
        std::cerr << "[Terrain NVIDIA] Falha ao compilar shaders!" << std::endl;
        return false;
    }

    m_bInitialized = true;

    std::cout << "[Terrain NVIDIA] Inicializado com sucesso!" << std::endl;
    std::cout << "[Terrain NVIDIA] Deferred: " << (m_Config.EnableDeferred ? "SIM" : "NÃO") << std::endl;
    std::cout << "[Terrain NVIDIA] Instancing: " << (m_Config.EnableInstancing ? "SIM" : "NÃO") << std::endl;
    std::cout << "[Terrain NVIDIA] LOD Distance: " << m_Config.LodDistance << std::endl;

    return true;
}

void CTerrainNvidia::Shutdown()
{
    if (!m_bInitialized)
        return;

    // Limpar chunks
    ClearAllChunks();

    // Limpar shaders
    if (m_ShaderProgram != 0)
    {
        glDeleteProgram(m_ShaderProgram);
        m_ShaderProgram = 0;
    }

    if (m_ComputeShader != 0)
    {
        glDeleteProgram(m_ComputeShader);
        m_ComputeShader = 0;
    }

    m_bInitialized = false;
    std::cout << "[Terrain NVIDIA] Finalizado!" << std::endl;
}

bool CTerrainNvidia::AddChunk(int x, int z, float size, GLuint terrainData)
{
    if (!m_bInitialized)
        return false;

    TerrainChunk chunk = {};
    chunk.Position = glm::vec3(x * size, 0.0f, z * size);
    chunk.Size = size;
    chunk.LODLevel = 0;
    chunk.IsVisible = true;

    // Criar VAO e VBO (simplificado)
    glGenVertexArrays(1, &chunk.VAO);
    glGenBuffers(1, &chunk.VBO);
    glGenBuffers(1, &chunk.EBO);

    // ... setup de buffers ...

    m_Chunks.push_back(chunk);

    std::cout << "[Terrain NVIDIA] Chunk adicionado em (" << x << ", " << z << ")" << std::endl;

    return true;
}

bool CTerrainNvidia::RemoveChunk(int x, int z)
{
    for (auto it = m_Chunks.begin(); it != m_Chunks.end(); ++it)
    {
        glm::vec3 expectedPos = glm::vec3(x * it->Size, 0.0f, z * it->Size);
        if (glm::distance(it->Position, expectedPos) < 0.1f)
        {
            glDeleteBuffers(1, &it->VBO);
            glDeleteBuffers(1, &it->EBO);
            glDeleteVertexArrays(1, &it->VAO);

            m_Chunks.erase(it);
            return true;
        }
    }
    return false;
}

void CTerrainNvidia::ClearAllChunks()
{
    for (auto& chunk : m_Chunks)
    {
        if (chunk.VAO != 0)
            glDeleteVertexArrays(1, &chunk.VAO);
        if (chunk.VBO != 0)
            glDeleteBuffers(1, &chunk.VBO);
        if (chunk.EBO != 0)
            glDeleteBuffers(1, &chunk.EBO);
    }
    m_Chunks.clear();
}

bool CTerrainNvidia::SetMaterial(int index, const TerrainMaterial& material)
{
    if (index < 0 || index >= (int)m_Materials.size())
    {
        m_Materials.resize(index + 1);
    }

    m_Materials[index] = material;
    return true;
}

const CTerrainNvidia::TerrainMaterial& CTerrainNvidia::GetMaterial(int index) const
{
    static TerrainMaterial dummy = {};
    
    if (index < 0 || index >= (int)m_Materials.size())
        return dummy;

    return m_Materials[index];
}

void CTerrainNvidia::Update(const glm::vec3& cameraPosition)
{
    if (!m_bInitialized)
        return;

    ResetStats();

    // Atualizar LOD de cada chunk
    UpdateChunkLOD();

    // Atualizar frustum culling
    glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
    glm::mat4 proj = glm::perspective(45.0f, 1024.0f / 768.0f, 0.1f, 1000.0f);
    UpdateFrustumCulling(proj * view);

    // Contar chunks visíveis
    m_Stats.TotalChunks = m_Chunks.size();
    for (const auto& chunk : m_Chunks)
    {
        if (chunk.IsVisible)
        {
            m_Stats.VisibleChunks++;
            m_Stats.VerticesRendered += chunk.VertexCount;
        }
    }

    // Usar compute shader se habilitado
    if (m_Config.EnableComputeShader && m_ComputeShader != 0)
    {
        UpdateComputeShader();
    }
}

void CTerrainNvidia::Render(const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
{
    if (!m_bInitialized || m_ShaderProgram == 0)
        return;

    glUseProgram(m_ShaderProgram);

    // Renderizar cada chunk visível
    for (auto& chunk : m_Chunks)
    {
        if (chunk.IsVisible)
        {
            RenderChunk(chunk, viewMatrix, projMatrix);
            m_Stats.DrawCalls++;
        }
    }

    glUseProgram(0);

    // Debug
    if (m_bDebugMode)
    {
        RenderDebugInfo();
    }
}

int CTerrainNvidia::GetLODLevel(const glm::vec3& chunkPos, const glm::vec3& cameraPos) const
{
    float distance = glm::distance(chunkPos, cameraPos);

    if (distance < m_Config.LodDistance * 0.25f)
        return 0;  // Máxima qualidade
    else if (distance < m_Config.LodDistance * 0.5f)
        return 1;
    else if (distance < m_Config.LodDistance * 0.75f)
        return 2;
    else
        return 3;  // Mínima qualidade
}

void CTerrainNvidia::UpdateFrustumCulling(const glm::mat4& viewProjMatrix)
{
    // Implementar frustum culling
    for (auto& chunk : m_Chunks)
    {
        // Simplificado - sempre visível por enquanto
        chunk.IsVisible = true;
    }
}

bool CTerrainNvidia::IsChunkVisible(const TerrainChunk& chunk) const
{
    return chunk.IsVisible;
}

void CTerrainNvidia::ResetStats()
{
    m_Stats.VisibleChunks = 0;
    m_Stats.VerticesRendered = 0;
    m_Stats.DrawCalls = 0;
}

void CTerrainNvidia::RenderDebugInfo()
{
    std::cout << "[Terrain NVIDIA] Stats:" << std::endl;
    std::cout << "  Chunks visíveis: " << m_Stats.VisibleChunks << "/" << m_Stats.TotalChunks << std::endl;
    std::cout << "  Vértices: " << m_Stats.VerticesRendered << std::endl;
    std::cout << "  Draw calls: " << m_Stats.DrawCalls << std::endl;
}

bool CTerrainNvidia::CompileShaders()
{
    // Shaders serão compilados quando necessário
    // Por enquanto, apenas placeholder
    return true;
}

void CTerrainNvidia::UpdateChunkLOD()
{
    // Atualizar LOD de cada chunk baseado na distância
    for (auto& chunk : m_Chunks)
    {
        // Simplicado
        chunk.LODLevel = 0;
    }
}

void CTerrainNvidia::RenderChunk(const TerrainChunk& chunk, const glm::mat4& viewMatrix, 
                                 const glm::mat4& projMatrix)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), chunk.Position);

    // Passar matrices ao shader
    GLint modelLoc = glGetUniformLocation(m_ShaderProgram, "uModel");
    GLint viewLoc = glGetUniformLocation(m_ShaderProgram, "uView");
    GLint projLoc = glGetUniformLocation(m_ShaderProgram, "uProj");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));

    // Renderizar
    glBindVertexArray(chunk.VAO);
    if (chunk.IndexCount > 0)
    {
        glDrawElements(GL_TRIANGLES, chunk.IndexCount, GL_UNSIGNED_INT, nullptr);
    }
    glBindVertexArray(0);
}

void CTerrainNvidia::UpdateComputeShader()
{
    // Executar compute shader para processamento de terreno
    if (m_ComputeShader == 0)
        return;

    glUseProgram(m_ComputeShader);
    glDispatchCompute((GLuint)m_Chunks.size(), 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glUseProgram(0);
}

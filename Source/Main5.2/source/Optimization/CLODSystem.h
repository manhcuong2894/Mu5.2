#pragma once

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

/**
 * @file CLODSystem.h
 * @brief Sistema de Level of Detail (LOD) para otimização de geometria
 */

enum class LODLevel
{
    ULTRA_HIGH = 0,   // Máxima qualidade (perto)
    HIGH = 1,         // Alta qualidade
    MEDIUM = 2,       // Qualidade média
    LOW = 3,          // Qualidade baixa
    ULTRA_LOW = 4     // Mínimo (distante)
};

struct LODMesh
{
    LODLevel Level;
    uint32_t VertexCount;
    uint32_t IndexCount;
    uint32_t VAO;
    uint32_t VBO;
    uint32_t IBO;
    float PixelThreshold;  // Quando mudar para este LOD
};

struct LODObject
{
    uint32_t ObjectID;
    std::vector<LODMesh> LODMeshes;
    LODLevel CurrentLOD;
    glm::vec3 Position;
    float BoundingRadius;
};

/**
 * @class CLODSystem
 * @brief Gerenciador de Level of Detail
 */
class CLODSystem
{
public:
    static CLODSystem* Instance();

    // Inicialização
    bool Initialize();
    void Shutdown();

    // Gerenciamento de objetos
    uint32_t RegisterObject(const glm::vec3& position, float radius);
    bool UnregisterObject(uint32_t objectID);
    void UpdateObjectPosition(uint32_t objectID, const glm::vec3& newPosition);

    // Malhas LOD
    bool AddLODMesh(uint32_t objectID, const LODMesh& mesh);
    LODMesh GetCurrentLODMesh(uint32_t objectID) const;

    // Atualização de LOD
    void UpdateLODLevel(uint32_t objectID, LODLevel level);
    void UpdateAllLODs(const glm::vec3& cameraPos, float cameraDistance);

    // Estatísticas
    uint32_t GetTotalVisibleTriangles() const { return m_VisibleTriangles; }
    uint32_t GetTotalVisibleVertices() const { return m_VisibleVertices; }
    uint32_t GetActiveObjects() const { return m_LODObjects.size(); }

    // Performance
    void OptimizeForDistance(float distance);
    float GetQualityScale() const { return m_QualityScale; }

    virtual ~CLODSystem();

private:
    CLODSystem();
    static CLODSystem* m_pInstance;

    bool m_bInitialized;
    uint32_t m_NextObjectID;

    std::vector<LODObject> m_LODObjects;
    uint32_t m_VisibleTriangles;
    uint32_t m_VisibleVertices;
    float m_QualityScale;

    LODLevel CalculateLODLevel(float distance, float pixelSize) const;
};

#define gLODSystem (CLODSystem::Instance())

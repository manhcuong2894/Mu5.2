#include "stdafx.h"
#include "CLODSystem.h"
#include <algorithm>

CLODSystem* CLODSystem::m_pInstance = nullptr;

CLODSystem::CLODSystem()
    : m_bInitialized(false),
      m_NextObjectID(1),
      m_VisibleTriangles(0),
      m_VisibleVertices(0),
      m_QualityScale(1.0f)
{
}

CLODSystem::~CLODSystem()
{
    Shutdown();
}

CLODSystem* CLODSystem::Instance()
{
    if (!m_pInstance)
        m_pInstance = new CLODSystem();
    return m_pInstance;
}

bool CLODSystem::Initialize()
{
    if (m_bInitialized)
        return true;

    m_bInitialized = true;
    return true;
}

void CLODSystem::Shutdown()
{
    if (!m_bInitialized)
        return;

    m_LODObjects.clear();
    m_bInitialized = false;
}

uint32_t CLODSystem::RegisterObject(const glm::vec3& position, float radius)
{
    LODObject obj;
    obj.ObjectID = m_NextObjectID++;
    obj.Position = position;
    obj.BoundingRadius = radius;
    obj.CurrentLOD = LODLevel::MEDIUM;

    m_LODObjects.push_back(obj);
    return obj.ObjectID;
}

bool CLODSystem::UnregisterObject(uint32_t objectID)
{
    auto it = std::find_if(m_LODObjects.begin(), m_LODObjects.end(),
        [objectID](const LODObject& obj) { return obj.ObjectID == objectID; });

    if (it == m_LODObjects.end())
        return false;

    m_LODObjects.erase(it);
    return true;
}

void CLODSystem::UpdateObjectPosition(uint32_t objectID, const glm::vec3& newPosition)
{
    auto it = std::find_if(m_LODObjects.begin(), m_LODObjects.end(),
        [objectID](const LODObject& obj) { return obj.ObjectID == objectID; });

    if (it != m_LODObjects.end())
    {
        it->Position = newPosition;
    }
}

bool CLODSystem::AddLODMesh(uint32_t objectID, const LODMesh& mesh)
{
    auto it = std::find_if(m_LODObjects.begin(), m_LODObjects.end(),
        [objectID](const LODObject& obj) { return obj.ObjectID == objectID; });

    if (it == m_LODObjects.end())
        return false;

    it->LODMeshes.push_back(mesh);
    return true;
}

LODMesh CLODSystem::GetCurrentLODMesh(uint32_t objectID) const
{
    auto it = std::find_if(m_LODObjects.begin(), m_LODObjects.end(),
        [objectID](const LODObject& obj) { return obj.ObjectID == objectID; });

    if (it == m_LODObjects.end())
        return LODMesh();

    // Encontrar mesh com o nível correto
    for (const auto& mesh : it->LODMeshes)
    {
        if (mesh.Level == it->CurrentLOD)
        {
            return mesh;
        }
    }

    return LODMesh();
}

void CLODSystem::UpdateLODLevel(uint32_t objectID, LODLevel level)
{
    auto it = std::find_if(m_LODObjects.begin(), m_LODObjects.end(),
        [objectID](const LODObject& obj) { return obj.ObjectID == objectID; });

    if (it != m_LODObjects.end())
    {
        it->CurrentLOD = level;
    }
}

void CLODSystem::UpdateAllLODs(const glm::vec3& cameraPos, float cameraDistance)
{
    m_VisibleTriangles = 0;
    m_VisibleVertices = 0;

    for (auto& obj : m_LODObjects)
    {
        float distance = glm::distance(obj.Position, cameraPos);
        
        // Calcular LOD baseado em distância
        LODLevel newLOD;
        if (distance < 20.0f)
            newLOD = LODLevel::ULTRA_HIGH;
        else if (distance < 50.0f)
            newLOD = LODLevel::HIGH;
        else if (distance < 100.0f)
            newLOD = LODLevel::MEDIUM;
        else if (distance < 200.0f)
            newLOD = LODLevel::LOW;
        else
            newLOD = LODLevel::ULTRA_LOW;

        obj.CurrentLOD = newLOD;

        // Contar triângulos/vértices visíveis
        for (const auto& mesh : obj.LODMeshes)
        {
            if (mesh.Level == obj.CurrentLOD)
            {
                m_VisibleTriangles += mesh.IndexCount / 3;
                m_VisibleVertices += mesh.VertexCount;
            }
        }
    }

    // Calcular quality scale
    const uint32_t maxTriangles = 5000000;
    m_QualityScale = (m_VisibleTriangles > 0) ? 
        (float)maxTriangles / (float)m_VisibleTriangles : 1.0f;
    m_QualityScale = (m_QualityScale > 1.0f) ? 1.0f : m_QualityScale;
}

void CLODSystem::OptimizeForDistance(float distance)
{
    // Ajustar LOD para distância específica
    for (auto& obj : m_LODObjects)
    {
        if (distance < 20.0f)
            obj.CurrentLOD = LODLevel::ULTRA_HIGH;
        else if (distance < 50.0f)
            obj.CurrentLOD = LODLevel::HIGH;
        else if (distance < 100.0f)
            obj.CurrentLOD = LODLevel::MEDIUM;
        else if (distance < 200.0f)
            obj.CurrentLOD = LODLevel::LOW;
        else
            obj.CurrentLOD = LODLevel::ULTRA_LOW;
    }
}

#pragma once

#include <glm/glm.hpp>
#include <vector>

/**
 * @file CViewFrustumCuller.h
 * @brief Sistema de culling de view frustum para otimizar renderização
 */

struct AABB  // Axis-Aligned Bounding Box
{
    glm::vec3 Min;
    glm::vec3 Max;
    
    bool IsValid() const { return glm::length(Max - Min) > 0.001f; }
};

struct BoundingSphere
{
    glm::vec3 Center;
    float Radius;
};

enum class CullResult
{
    INSIDE = 0,      // Completamente dentro do frustum
    OUTSIDE = 1,     // Completamente fora
    INTERSECTING = 2 // Parcialmente dentro
};

/**
 * @class CViewFrustumCuller
 * @brief Culling de view frustum para otimização
 */
class CViewFrustumCuller
{
public:
    static CViewFrustumCuller* Instance();

    // Inicialização
    bool Initialize();
    void Shutdown();

    // Atualizar frustum
    void UpdateFrustum(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void SetViewFrustum(float fov, float aspect, float nearPlane, float farPlane);

    // Testes de culling
    CullResult TestAABB(const AABB& box) const;
    CullResult TestSphere(const BoundingSphere& sphere) const;
    CullResult TestPoint(const glm::vec3& point) const;

    // Batch culling
    void CullObjects(const std::vector<AABB>& objects, std::vector<bool>& results);
    void CullSpheres(const std::vector<BoundingSphere>& spheres, std::vector<bool>& results);

    // Estatísticas
    uint32_t GetCulledObjectCount() const { return m_CulledObjectCount; }
    uint32_t GetVisibleObjectCount() const { return m_VisibleObjectCount; }
    float GetCullRatio() const;

    // Debug
    void ToggleDebugVisualization(bool enable);
    void PrintFrustumInfo() const;

    virtual ~CViewFrustumCuller();

private:
    CViewFrustumCuller();
    static CViewFrustumCuller* m_pInstance;

    bool m_bInitialized;
    bool m_bDebugVisualization;

    // Frustum planes
    glm::vec4 m_FrustumPlanes[6];  // left, right, top, bottom, near, far

    // Estatísticas
    uint32_t m_CulledObjectCount;
    uint32_t m_VisibleObjectCount;

    // Métodos privados
    void ExtractFrustumPlanes(const glm::mat4& viewProj);
    float SignedDistance(const glm::vec3& point, const glm::vec4& plane) const;
};

#define gViewFrustumCuller (CViewFrustumCuller::Instance())

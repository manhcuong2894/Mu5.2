#include "stdafx.h"
#include "CViewFrustumCuller.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

CViewFrustumCuller* CViewFrustumCuller::m_pInstance = nullptr;

CViewFrustumCuller::CViewFrustumCuller()
    : m_bInitialized(false),
      m_bDebugVisualization(false),
      m_CulledObjectCount(0),
      m_VisibleObjectCount(0)
{
}

CViewFrustumCuller::~CViewFrustumCuller()
{
    Shutdown();
}

CViewFrustumCuller* CViewFrustumCuller::Instance()
{
    if (!m_pInstance)
        m_pInstance = new CViewFrustumCuller();
    return m_pInstance;
}

bool CViewFrustumCuller::Initialize()
{
    if (m_bInitialized)
        return true;

    m_bInitialized = true;
    return true;
}

void CViewFrustumCuller::Shutdown()
{
    m_bInitialized = false;
}

void CViewFrustumCuller::UpdateFrustum(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    glm::mat4 viewProj = projectionMatrix * viewMatrix;
    ExtractFrustumPlanes(viewProj);
}

void CViewFrustumCuller::SetViewFrustum(float fov, float aspect, float nearPlane, float farPlane)
{
    glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    glm::mat4 view = glm::mat4(1.0f);
    UpdateFrustum(view, proj);
}

CullResult CViewFrustumCuller::TestAABB(const AABB& box) const
{
    bool allInside = true;

    for (int i = 0; i < 6; i++)
    {
        glm::vec3 p = box.Min;
        if (m_FrustumPlanes[i].x >= 0) p.x = box.Max.x;
        if (m_FrustumPlanes[i].y >= 0) p.y = box.Max.y;
        if (m_FrustumPlanes[i].z >= 0) p.z = box.Max.z;

        float distance = SignedDistance(p, m_FrustumPlanes[i]);
        if (distance < 0)
            return CullResult::OUTSIDE;

        p = box.Max;
        if (m_FrustumPlanes[i].x >= 0) p.x = box.Min.x;
        if (m_FrustumPlanes[i].y >= 0) p.y = box.Min.y;
        if (m_FrustumPlanes[i].z >= 0) p.z = box.Min.z;

        if (SignedDistance(p, m_FrustumPlanes[i]) < 0)
            allInside = false;
    }

    return allInside ? CullResult::INSIDE : CullResult::INTERSECTING;
}

CullResult CViewFrustumCuller::TestSphere(const BoundingSphere& sphere) const
{
    bool allInside = true;

    for (int i = 0; i < 6; i++)
    {
        float distance = SignedDistance(sphere.Center, m_FrustumPlanes[i]);

        if (distance < -sphere.Radius)
            return CullResult::OUTSIDE;

        if (distance < sphere.Radius)
            allInside = false;
    }

    return allInside ? CullResult::INSIDE : CullResult::INTERSECTING;
}

CullResult CViewFrustumCuller::TestPoint(const glm::vec3& point) const
{
    for (int i = 0; i < 6; i++)
    {
        if (SignedDistance(point, m_FrustumPlanes[i]) < 0)
            return CullResult::OUTSIDE;
    }
    return CullResult::INSIDE;
}

void CViewFrustumCuller::CullObjects(const std::vector<AABB>& objects, std::vector<bool>& results)
{
    results.resize(objects.size());
    m_VisibleObjectCount = 0;
    m_CulledObjectCount = 0;

    for (size_t i = 0; i < objects.size(); i++)
    {
        CullResult result = TestAABB(objects[i]);
        results[i] = (result != CullResult::OUTSIDE);

        if (results[i])
            m_VisibleObjectCount++;
        else
            m_CulledObjectCount++;
    }
}

void CViewFrustumCuller::CullSpheres(const std::vector<BoundingSphere>& spheres, std::vector<bool>& results)
{
    results.resize(spheres.size());
    m_VisibleObjectCount = 0;
    m_CulledObjectCount = 0;

    for (size_t i = 0; i < spheres.size(); i++)
    {
        CullResult result = TestSphere(spheres[i]);
        results[i] = (result != CullResult::OUTSIDE);

        if (results[i])
            m_VisibleObjectCount++;
        else
            m_CulledObjectCount++;
    }
}

float CViewFrustumCuller::GetCullRatio() const
{
    uint32_t total = m_VisibleObjectCount + m_CulledObjectCount;
    if (total == 0)
        return 0.0f;

    return (float)m_CulledObjectCount / (float)total;
}

void CViewFrustumCuller::ToggleDebugVisualization(bool enable)
{
    m_bDebugVisualization = enable;
}

void CViewFrustumCuller::PrintFrustumInfo() const
{
}

void CViewFrustumCuller::ExtractFrustumPlanes(const glm::mat4& viewProj)
{
    m_FrustumPlanes[0] = glm::normalize(
        glm::vec4(viewProj[0][3] + viewProj[0][0],
                  viewProj[1][3] + viewProj[1][0],
                  viewProj[2][3] + viewProj[2][0],
                  viewProj[3][3] + viewProj[3][0]));

    m_FrustumPlanes[1] = glm::normalize(
        glm::vec4(viewProj[0][3] - viewProj[0][0],
                  viewProj[1][3] - viewProj[1][0],
                  viewProj[2][3] - viewProj[2][0],
                  viewProj[3][3] - viewProj[3][0]));

    m_FrustumPlanes[2] = glm::normalize(
        glm::vec4(viewProj[0][3] + viewProj[0][1],
                  viewProj[1][3] + viewProj[1][1],
                  viewProj[2][3] + viewProj[2][1],
                  viewProj[3][3] + viewProj[3][1]));

    m_FrustumPlanes[3] = glm::normalize(
        glm::vec4(viewProj[0][3] - viewProj[0][1],
                  viewProj[1][3] - viewProj[1][1],
                  viewProj[2][3] - viewProj[2][1],
                  viewProj[3][3] - viewProj[3][1]));

    m_FrustumPlanes[4] = glm::normalize(
        glm::vec4(viewProj[0][3] + viewProj[0][2],
                  viewProj[1][3] + viewProj[1][2],
                  viewProj[2][3] + viewProj[2][2],
                  viewProj[3][3] + viewProj[3][2]));

    m_FrustumPlanes[5] = glm::normalize(
        glm::vec4(viewProj[0][3] - viewProj[0][2],
                  viewProj[1][3] - viewProj[1][2],
                  viewProj[2][3] - viewProj[2][2],
                  viewProj[3][3] - viewProj[3][2]));
}

float CViewFrustumCuller::SignedDistance(const glm::vec3& point, const glm::vec4& plane) const
{
    return glm::dot(glm::vec3(plane), point) + plane.w;
}

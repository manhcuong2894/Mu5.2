#include "stdafx.h"
#include "CCameraOptimizer.h"
#include <algorithm>
#include <cmath>

CCameraOptimizer* CCameraOptimizer::m_pInstance = nullptr;

CCameraOptimizer::CCameraOptimizer()
    : m_bInitialized(false),
      m_CameraPosition(0.0f),
      m_CameraLookAt(0.0f),
      m_CameraDistance(50.0f),
      m_CameraFOV(45.0f),
      m_ZoomVelocity(0.0f),
      m_bZoomingIn(false),
      m_bZoomingOut(false),
      m_PreviousDistance(50.0f),
      m_CurrentZoomLevel(CameraZoomLevel::MEDIUM),
      m_PreviousZoomLevel(CameraZoomLevel::MEDIUM)
{
}

CCameraOptimizer::~CCameraOptimizer()
{
    Shutdown();
}

CCameraOptimizer* CCameraOptimizer::Instance()
{
    if (!m_pInstance)
        m_pInstance = new CCameraOptimizer();
    return m_pInstance;
}

bool CCameraOptimizer::Initialize()
{
    if (m_bInitialized)
        return true;

    m_ViewMatrix = glm::mat4(1.0f);
    m_ProjectionMatrix = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 1000.0f);
    m_ViewProjection = m_ProjectionMatrix * m_ViewMatrix;

    GenerateQualityConfig(m_CurrentZoomLevel, m_CurrentQualityConfig);

    m_bInitialized = true;
    return true;
}

void CCameraOptimizer::Shutdown()
{
    if (!m_bInitialized)
        return;

    m_bInitialized = false;
}

void CCameraOptimizer::UpdateCameraDistance(float distance)
{
    m_PreviousDistance = m_CameraDistance;
    m_CameraDistance = distance;

    m_ZoomVelocity = m_CameraDistance - m_PreviousDistance;
    m_bZoomingIn = m_ZoomVelocity > 0.1f;
    m_bZoomingOut = m_ZoomVelocity < -0.1f;

    DetectZoomChange();
    UpdateFrustumPlanes();
}

void CCameraOptimizer::SetCameraPosition(const glm::vec3& position)
{
    m_CameraPosition = position;
    UpdateFrustumPlanes();
}

void CCameraOptimizer::SetCameraFOV(float fov)
{
    m_CameraFOV = fov;
    m_ProjectionMatrix = glm::perspective(glm::radians(fov), 16.0f / 9.0f, 0.1f, 1000.0f);
    UpdateFrustumPlanes();
}

void CCameraOptimizer::AdaptGeometryQuality()
{
    switch (m_CurrentZoomLevel)
    {
        case CameraZoomLevel::VERY_CLOSE:
            m_CurrentQualityConfig.MaxGeometryTriangles = 5000000;
            m_CurrentQualityConfig.DetailLODLevel = 0;
            m_CurrentQualityConfig.EnableDetailGeometry = true;
            break;

        case CameraZoomLevel::CLOSE:
            m_CurrentQualityConfig.MaxGeometryTriangles = 3000000;
            m_CurrentQualityConfig.DetailLODLevel = 1;
            break;

        case CameraZoomLevel::MEDIUM:
            m_CurrentQualityConfig.MaxGeometryTriangles = 2000000;
            m_CurrentQualityConfig.DetailLODLevel = 2;
            break;

        case CameraZoomLevel::ZOOM_FAR:
            m_CurrentQualityConfig.MaxGeometryTriangles = 1000000;
            m_CurrentQualityConfig.DetailLODLevel = 3;
            m_CurrentQualityConfig.EnableDetailGeometry = false;
            break;

        case CameraZoomLevel::VERY_FAR:
            m_CurrentQualityConfig.MaxGeometryTriangles = 500000;
            m_CurrentQualityConfig.DetailLODLevel = 4;
            break;

        case CameraZoomLevel::EXTREME:
            m_CurrentQualityConfig.MaxGeometryTriangles = 200000;
            m_CurrentQualityConfig.DetailLODLevel = 5;
            m_CurrentQualityConfig.EnableDetailGeometry = false;
            break;
    }

    ApplyQualityAdaptations();
}

void CCameraOptimizer::AdaptShaderQuality()
{
    switch (m_CurrentZoomLevel)
    {
        case CameraZoomLevel::VERY_CLOSE:
            m_CurrentQualityConfig.EnableComplexShaders = true;
            m_CurrentQualityConfig.EnableParallaxMapping = true;
            m_CurrentQualityConfig.EnableNormalMapping = true;
            m_CurrentQualityConfig.EnableSpecularMapping = true;
            m_CurrentQualityConfig.ShaderQualityLevel = 3;
            break;

        case CameraZoomLevel::CLOSE:
            m_CurrentQualityConfig.ShaderQualityLevel = 3;
            m_CurrentQualityConfig.EnableParallaxMapping = true;
            break;

        case CameraZoomLevel::MEDIUM:
            m_CurrentQualityConfig.ShaderQualityLevel = 2;
            m_CurrentQualityConfig.EnableParallaxMapping = false;
            break;

        case CameraZoomLevel::ZOOM_FAR:
            m_CurrentQualityConfig.ShaderQualityLevel = 2;
            m_CurrentQualityConfig.EnableNormalMapping = false;
            break;

        case CameraZoomLevel::VERY_FAR:
            m_CurrentQualityConfig.ShaderQualityLevel = 1;
            m_CurrentQualityConfig.EnableComplexShaders = false;
            break;

        case CameraZoomLevel::EXTREME:
            m_CurrentQualityConfig.ShaderQualityLevel = 0;
            m_CurrentQualityConfig.EnableComplexShaders = false;
            m_CurrentQualityConfig.EnableSpecularMapping = false;
            break;
    }
}

void CCameraOptimizer::AdaptEffectQuality()
{
    switch (m_CurrentZoomLevel)
    {
        case CameraZoomLevel::VERY_CLOSE:
        case CameraZoomLevel::CLOSE:
            m_CurrentQualityConfig.EnableParticles = true;
            m_CurrentQualityConfig.EnableGlow = true;
            m_CurrentQualityConfig.EnableSSAO = true;
            m_CurrentQualityConfig.EnableSSR = true;
            m_CurrentQualityConfig.MaxParticles = 10000;
            break;

        case CameraZoomLevel::MEDIUM:
            m_CurrentQualityConfig.EnableSSAO = true;
            m_CurrentQualityConfig.EnableSSR = false;
            m_CurrentQualityConfig.MaxParticles = 5000;
            break;

        case CameraZoomLevel::ZOOM_FAR:
            m_CurrentQualityConfig.EnableParticles = false;
            m_CurrentQualityConfig.EnableGlow = false;
            m_CurrentQualityConfig.EnableSSAO = false;
            m_CurrentQualityConfig.MaxParticles = 1000;
            break;

        case CameraZoomLevel::VERY_FAR:
        case CameraZoomLevel::EXTREME:
            m_CurrentQualityConfig.EnableParticles = false;
            m_CurrentQualityConfig.EnableGlow = false;
            m_CurrentQualityConfig.EnableSSAO = false;
            m_CurrentQualityConfig.EnableSSR = false;
            m_CurrentQualityConfig.MaxParticles = 0;
            break;
    }
}

void CCameraOptimizer::AdaptScaleformQuality()
{
    switch (m_CurrentZoomLevel)
    {
        case CameraZoomLevel::EXTREME:
            m_CurrentQualityConfig.EnableScaleformEffects = false;
            m_CurrentQualityConfig.ScaleformQuality = 0;
            m_CurrentQualityConfig.RenderScaleformOffscreen = true;
            break;

        case CameraZoomLevel::VERY_FAR:
            m_CurrentQualityConfig.ScaleformQuality = 1;
            m_CurrentQualityConfig.RenderScaleformOffscreen = false;
            break;

        default:
            m_CurrentQualityConfig.EnableScaleformEffects = true;
            m_CurrentQualityConfig.ScaleformQuality = 3;
            m_CurrentQualityConfig.RenderScaleformOffscreen = false;
            break;
    }
}

void CCameraOptimizer::AdaptCPUGPULoad()
{
    switch (m_CurrentZoomLevel)
    {
        case CameraZoomLevel::VERY_CLOSE:
        case CameraZoomLevel::CLOSE:
            m_CurrentQualityConfig.BatchSizeMultiplier = 1.0f;
            m_CurrentQualityConfig.EnableAsyncCompute = true;
            m_CurrentQualityConfig.EnableACEQueues = true;
            break;

        case CameraZoomLevel::MEDIUM:
            m_CurrentQualityConfig.BatchSizeMultiplier = 0.8f;
            m_CurrentQualityConfig.EnableAsyncCompute = true;
            break;

        case CameraZoomLevel::ZOOM_FAR:
        case CameraZoomLevel::VERY_FAR:
            m_CurrentQualityConfig.BatchSizeMultiplier = 0.6f;
            m_CurrentQualityConfig.EnableAsyncCompute = false;
            break;

        case CameraZoomLevel::EXTREME:
            m_CurrentQualityConfig.BatchSizeMultiplier = 0.4f;
            m_CurrentQualityConfig.EnableAsyncCompute = false;
            m_CurrentQualityConfig.EnableACEQueues = false;
            break;
    }
}

bool CCameraOptimizer::IsObjectInView(const glm::vec3& position, float radius) const
{
    return SphereInFrustum(position, radius);
}

bool CCameraOptimizer::IsTerrainChunkVisible(const glm::vec3& chunkCenter, float chunkSize) const
{
    float radius = chunkSize * 0.866f;
    return SphereInFrustum(chunkCenter, radius);
}

void CCameraOptimizer::PrintCameraInfo() const
{
}

void CCameraOptimizer::ToggleFrustumVisualization(bool enable)
{
}

void CCameraOptimizer::DetectZoomChange()
{
    CameraZoomLevel newLevel = CalculateZoomLevel(m_CameraDistance);

    if (newLevel != m_CurrentZoomLevel)
    {
        m_PreviousZoomLevel = m_CurrentZoomLevel;
        m_CurrentZoomLevel = newLevel;

        GenerateQualityConfig(m_CurrentZoomLevel, m_CurrentQualityConfig);
        ApplyQualityAdaptations();
    }
}

CameraZoomLevel CCameraOptimizer::CalculateZoomLevel(float distance)
{
    if (distance < 20.0f)
        return CameraZoomLevel::VERY_CLOSE;
    else if (distance < 50.0f)
        return CameraZoomLevel::CLOSE;
    else if (distance < 100.0f)
        return CameraZoomLevel::MEDIUM;
    else if (distance < 200.0f)
        return CameraZoomLevel::ZOOM_FAR;
    else if (distance < 500.0f)
        return CameraZoomLevel::VERY_FAR;
    else
        return CameraZoomLevel::EXTREME;
}

void CCameraOptimizer::GenerateQualityConfig(CameraZoomLevel level, CameraQualityConfig& config)
{
    config.Level = level;
    config.CameraDistance = m_CameraDistance;

    config.EnableComplexShaders = true;
    config.EnableParallaxMapping = true;
    config.EnableNormalMapping = true;
    config.EnableSpecularMapping = true;
    config.EnableParticles = true;
    config.EnableGlow = true;
    config.EnableSSAO = true;
    config.EnableSSR = true;
    config.EnableDetailGeometry = true;
    config.EnableOcclusionCulling = true;
    config.EnableScaleformEffects = true;
    config.RenderScaleformOffscreen = false;

    config.ViewFrustumNear = 0.1f;
    config.ViewFrustumFar = 1000.0f;

    AdaptGeometryQuality();
    AdaptShaderQuality();
    AdaptEffectQuality();
    AdaptScaleformQuality();
    AdaptCPUGPULoad();
}

void CCameraOptimizer::UpdateFrustumPlanes()
{
    glm::mat4 viewProj = m_ProjectionMatrix * m_ViewMatrix;

    for (int i = 0; i < 4; i++)
    {
        m_FrustumPlanes[i].x = viewProj[0][3] + (i < 2 ? viewProj[0][i / 2] : -viewProj[0][i - 2]);
        m_FrustumPlanes[i].y = viewProj[1][3] + (i < 2 ? viewProj[1][i / 2] : -viewProj[1][i - 2]);
        m_FrustumPlanes[i].z = viewProj[2][3] + (i < 2 ? viewProj[2][i / 2] : -viewProj[2][i - 2]);
        m_FrustumPlanes[i].w = viewProj[3][3] + (i < 2 ? viewProj[3][i / 2] : -viewProj[3][i - 2]);

        float length = glm::length(glm::vec3(m_FrustumPlanes[i]));
        m_FrustumPlanes[i] /= length;
    }

    m_FrustumPlanes[4] = glm::normalize(glm::vec4(viewProj[0][2], viewProj[1][2], viewProj[2][2], viewProj[3][2]));
    m_FrustumPlanes[5] = glm::normalize(glm::vec4(viewProj[0][3] - viewProj[0][2], viewProj[1][3] - viewProj[1][2], 
                                                   viewProj[2][3] - viewProj[2][2], viewProj[3][3] - viewProj[3][2]));
}

void CCameraOptimizer::ApplyQualityAdaptations()
{
}

bool CCameraOptimizer::PointInFrustum(const glm::vec3& point) const
{
    for (int i = 0; i < 6; i++)
    {
        float distance = glm::dot(glm::vec3(m_FrustumPlanes[i]), point) + m_FrustumPlanes[i].w;
        if (distance < 0)
            return false;
    }
    return true;
}

bool CCameraOptimizer::SphereInFrustum(const glm::vec3& center, float radius) const
{
    for (int i = 0; i < 6; i++)
    {
        float distance = glm::dot(glm::vec3(m_FrustumPlanes[i]), center) + m_FrustumPlanes[i].w;
        if (distance < -radius)
            return false;
    }
    return true;
}

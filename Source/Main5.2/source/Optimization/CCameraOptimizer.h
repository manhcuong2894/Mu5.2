#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <map>

/**
 * @file CCameraOptimizer.h
 * @brief Sistema de otimização de renderização baseado em distância da câmera
 */

enum class CameraZoomLevel : uint8_t
{
    VERY_CLOSE = 0,
    CLOSE = 1,
    MEDIUM = 2,
    ZOOM_FAR = 3,
    VERY_FAR = 4,
    EXTREME = 5
};

struct CameraQualityConfig
{
    CameraZoomLevel Level;
    float CameraDistance;
    uint32_t MaxGeometryTriangles;
    bool EnableDetailGeometry;
    uint32_t DetailLODLevel;
    bool EnableComplexShaders;
    bool EnableParallaxMapping;
    bool EnableNormalMapping;
    bool EnableSpecularMapping;
    uint32_t ShaderQualityLevel;
    bool EnableParticles;
    bool EnableGlow;
    bool EnableSSAO;
    bool EnableSSR;
    uint32_t MaxParticles;
    bool EnableAsyncCompute;
    bool EnableACEQueues;
    float BatchSizeMultiplier;
    float ViewFrustumNear;
    float ViewFrustumFar;
    bool EnableOcclusionCulling;
    bool EnableScaleformEffects;
    uint32_t ScaleformQuality;
    bool RenderScaleformOffscreen;
};

class CCameraOptimizer
{
public:
    static CCameraOptimizer* Instance();

    bool Initialize();
    void Shutdown();

    void UpdateCameraDistance(float distance);
    void SetCameraPosition(const glm::vec3& position);
    void SetCameraFOV(float fov);

    CameraZoomLevel GetCurrentZoomLevel() const { return m_CurrentZoomLevel; }
    CameraQualityConfig GetQualityConfig() const { return m_CurrentQualityConfig; }
    float GetCameraDistance() const { return m_CameraDistance; }

    bool IsZoomingIn() const { return m_bZoomingIn; }
    bool IsZoomingOut() const { return m_bZoomingOut; }
    float GetZoomVelocity() const { return m_ZoomVelocity; }

    void AdaptGeometryQuality();
    void AdaptShaderQuality();
    void AdaptEffectQuality();
    void AdaptScaleformQuality();
    void AdaptCPUGPULoad();

    bool IsObjectInView(const glm::vec3& position, float radius) const;
    bool IsTerrainChunkVisible(const glm::vec3& chunkCenter, float chunkSize) const;

    struct CameraStats
    {
        float CameraDistance;
        CameraZoomLevel ZoomLevel;
        uint32_t VisibleTriangles;
        uint32_t VisibleObjects;
        float QualityScale;
        float CPULoadPercent;
        float GPULoadPercent;
    };

    CameraStats GetStats() const { return m_Stats; }

    void PrintCameraInfo() const;
    void ToggleFrustumVisualization(bool enable);

    virtual ~CCameraOptimizer();

private:
    CCameraOptimizer();
    static CCameraOptimizer* m_pInstance;

    bool m_bInitialized;
    glm::vec3 m_CameraPosition;
    glm::vec3 m_CameraLookAt;
    float m_CameraDistance;
    float m_CameraFOV;
    float m_ZoomVelocity;
    bool m_bZoomingIn;
    bool m_bZoomingOut;
    float m_PreviousDistance;

    CameraZoomLevel m_CurrentZoomLevel;
    CameraZoomLevel m_PreviousZoomLevel;
    CameraQualityConfig m_CurrentQualityConfig;

    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ProjectionMatrix;
    glm::mat4 m_ViewProjection;
    glm::vec4 m_FrustumPlanes[6];

    CameraStats m_Stats;

    void DetectZoomChange();
    CameraZoomLevel CalculateZoomLevel(float distance);
    void GenerateQualityConfig(CameraZoomLevel level, CameraQualityConfig& config);
    void UpdateFrustumPlanes();
    void ApplyQualityAdaptations();
    bool PointInFrustum(const glm::vec3& point) const;
    bool SphereInFrustum(const glm::vec3& center, float radius) const;
};

#define gCameraOptimizer (CCameraOptimizer::Instance())

#pragma once

/**
 * @file TerrainNvidiaConfig.h
 * @brief Configurações de renderização NVIDIA para Terrain
 * @details Parâmetros otimizados para renderização de terreno com GPU
 */

namespace TerrainNvidia
{
    // Estrutura de configuração
    struct Config
    {
        // Renderização
        bool EnableDeferred;                // Usar renderização deferred
        bool EnableInstancing;              // Usar instancing para LOD
        bool EnableAsyncCompute;            // Computação assíncrona
        
        // Otimizações
        bool EnableTessellation;            // Tessellation de terrain
        bool EnableGeometryShader;          // Geometry shaders
        bool EnableComputeShader;           // Compute shaders
        
        // Qualidade
        float TerrainDetailLevel;           // 0.0 = baixo, 1.0 = alto
        int MaxVerticesPerChunk;            // Vértices por chunk
        float LodDistance;                  // Distância de LOD
        
        // Performance
        bool EnableGPUCulling;              // Culling na GPU
        bool EnableVirtualTexturing;        // Virtual texturing
        int MaxTexturesPerMaterial;         // Texturas simultâneas
        
        // Debug
        bool ShowLodLevels;                 // Visualizar LOD levels
        bool ShowGPUStats;                  // Mostrar stats GPU
        bool WireframeMode;                 // Modo wireframe
    };

    // Configuração padrão otimizada
    static const Config DEFAULT_CONFIG = {
        true,       // EnableDeferred
        true,       // EnableInstancing
        true,       // EnableAsyncCompute
        true,       // EnableTessellation
        true,       // EnableGeometryShader
        true,       // EnableComputeShader
        1.0f,       // TerrainDetailLevel (máximo)
        32768,      // MaxVerticesPerChunk
        300.0f,     // LodDistance
        true,       // EnableGPUCulling
        true,       // EnableVirtualTexturing
        8,          // MaxTexturesPerMaterial
        false,      // ShowLodLevels
        false,      // ShowGPUStats
        false       // WireframeMode
    };

    // Configuração para performance
    static const Config PERFORMANCE_CONFIG = {
        false,      // EnableDeferred (menos quality)
        true,       // EnableInstancing
        true,       // EnableAsyncCompute
        false,      // EnableTessellation
        false,      // EnableGeometryShader
        true,       // EnableComputeShader
        0.5f,       // TerrainDetailLevel (médio)
        16384,      // MaxVerticesPerChunk
        200.0f,     // LodDistance
        true,       // EnableGPUCulling
        false,      // EnableVirtualTexturing
        4,          // MaxTexturesPerMaterial
        false,      // ShowLodLevels
        false,      // ShowGPUStats
        false       // WireframeMode
    };

    // Configuração para qualidade máxima
    static const Config QUALITY_CONFIG = {
        true,       // EnableDeferred
        true,       // EnableInstancing
        true,       // EnableAsyncCompute
        true,       // EnableTessellation
        true,       // EnableGeometryShader
        true,       // EnableComputeShader
        1.0f,       // TerrainDetailLevel (máximo)
        65536,      // MaxVerticesPerChunk
        500.0f,     // LodDistance
        true,       // EnableGPUCulling
        true,       // EnableVirtualTexturing
        16,         // MaxTexturesPerMaterial
        false,      // ShowLodLevels
        false,      // ShowGPUStats
        false       // WireframeMode
    };
}

#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>

/**
 * @brief Sistema de Debug para NVIDIA
 * @details Monitoramento visual e console da utilização de GPU
 */
class CNvidiaDebugMonitor
{
public:
    // Estrutura para estatísticas detalhadas
    struct DetailedStats
    {
        // GPU
        float gpuUtilization;           // 0-100%
        float gpuMemoryUsage;           // 0-100%
        size_t gpuMemoryUsedMB;         // MB usado
        size_t gpuMemoryTotalMB;        // MB total
        
        // Renderização
        int drawCalls;                  // Número de draw calls
        int trianglesRendered;          // Triângulos renderizados
        float fps;                      // Frames por segundo
        float frameTimeMS;              // Tempo de frame em ms
        
        // Shader
        bool shaderActive;              // Shader ativo?
        GLuint activeShaderProgram;     // Shader atual
        
        // Memory
        size_t bufferMemory;            // Memória de buffers
        size_t textureMemory;           // Memória de texturas
        
        // Status
        bool isGPURenderingActive;      // GPU renderizando?
        bool isNvidiaDetected;          // NVIDIA disponível?
        const char* gpuName;            // Nome da GPU
    };

    static CNvidiaDebugMonitor* Instance();

    // Inicialização
    bool Initialize();
    void Shutdown();

    // Update - chamar a cada frame
    void UpdateStats();

    // Getters
    const DetailedStats& GetStats() const { return m_Stats; }
    float GetGPUUtilization() const { return m_Stats.gpuUtilization; }
    float GetGPUMemoryUsage() const { return m_Stats.gpuMemoryUsage; }
    float GetFPS() const { return m_Stats.fps; }
    float GetFrameTime() const { return m_Stats.frameTimeMS; }
    int GetDrawCalls() const { return m_Stats.drawCalls; }
    bool IsGPUActive() const { return m_Stats.isGPURenderingActive; }

    // Debug Output
    void PrintStatsToConsole();
    void LogStatsToFile(const char* filename);
    
    // Render debug HUD (opcional)
    void RenderDebugOverlay();
    void RenderPerformanceGraph();
    
    // Controle
    void EnableDebugOutput(bool enable) { m_bDebugEnabled = enable; }
    void EnableHUDOverlay(bool enable) { m_bShowHUD = enable; }
    void SetUpdateInterval(float seconds) { m_UpdateInterval = seconds; }

    virtual ~CNvidiaDebugMonitor();

private:
    CNvidiaDebugMonitor();
    
    static CNvidiaDebugMonitor* m_pInstance;
    
    DetailedStats m_Stats;
    DetailedStats m_PreviousStats;
    
    bool m_bInitialized;
    bool m_bDebugEnabled;
    bool m_bShowHUD;
    
    float m_UpdateInterval;
    float m_TimeSinceLastUpdate;
    float m_FrameTimeBuffer[60];  // Buffer para gráfico
    int m_FrameTimeIndex;
    
    // Private methods
    void UpdateGPUStats();
    void UpdateRenderStats();
    void UpdateMemoryStats();
    void CalculateFPS();
};

#define gNvidiaDebug (CNvidiaDebugMonitor::Instance())

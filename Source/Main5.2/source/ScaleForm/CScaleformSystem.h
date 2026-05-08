#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

/**
 * @file CScaleformSystem.h
 * @brief Sistema nativo de Scaleform (.gfx) para MU Online Season 6
 * @details Carregamento, renderização e gerenciamento de arquivos Flash (.gfx)
 * 
 * Características:
 * - Carregamento nativo de arquivos .gfx (sem DLLs de terceiros)
 * - Cache de recursos em memória
 * - Renderização otimizada com OpenGL
 * - Sistema de eventos e callbacks
 * - Suporte a ActionScript (interface C++)
 * - Gerenciamento automático de memória
 * - Suporte a múltiplas instâncias simultâneas
 */

// Tipos de dados Scaleform
typedef uint32_t GFxHandle;
typedef void* GFxObject;

// Estados de carregamento
enum class GFxLoadState
{
    UNLOADED = 0,
    LOADING = 1,
    LOADED = 2,
    LOAD_ERROR = 3
};

// Estrutura de dados de arquivo GFX
struct GFxFileHeader
{
    char Signature[3];      // "FWS" ou "CWS" (comprimido)
    uint8_t Version;
    uint32_t FrameSize;
    uint16_t FrameRate;
    uint16_t FrameCount;
    uint32_t FileSize;
    
    bool IsValid() const
    {
        return (Signature[0] == 'F' || Signature[0] == 'C') &&
               Signature[1] == 'W' && Signature[2] == 'S' &&
               Version >= 5 && Version <= 15;
    }
};

// Estrutura de metadados do filme
struct GFxMovieInfo
{
    std::string FileName;
    GFxLoadState LoadState;
    GFxFileHeader Header;
    uint32_t MemoryUsage;
    float CurrentFrame;
    bool IsPlaying;
    glm::uvec2 Resolution;
    std::string LastError;
};

// Struct para evento de callback
struct GFxCallbackEvent
{
    GFxHandle MovieHandle;
    std::string FunctionName;
    std::vector<std::string> Arguments;
    bool IsHandled;
};

/**
 * @class CScaleformSystem
 * @brief Sistema central de gerenciamento de Scaleform
 */
class CScaleformSystem
{
public:
    // Tipos de textura suportadas
    enum class TextureFormat
    {
        RGBA8,
        RGB8,
        DXT1,
        DXT5,
        BGRA8
    };

    static CScaleformSystem* Instance();

    // Inicialização e shutdown
    bool Initialize();
    void Shutdown();
    bool IsInitialized() const { return m_bInitialized; }

    // Carregamento de arquivos .gfx
    GFxHandle LoadMovie(const std::string& filePath, uint32_t targetWidth = 1280, uint32_t targetHeight = 720);
    bool UnloadMovie(GFxHandle handle);
    void UnloadAllMovies();

    // Gerenciamento de filmes
    bool IsMovieLoaded(GFxHandle handle) const;
    GFxMovieInfo GetMovieInfo(GFxHandle handle) const;
    GFxHandle FindMovieByName(const std::string& movieName) const;
    const std::vector<GFxHandle>& GetLoadedMovies() const { return m_LoadedMovies; }

    // Controle de reprodução
    void PlayMovie(GFxHandle handle);
    void PauseMovie(GFxHandle handle);
    void StopMovie(GFxHandle handle);
    void SetMovieFrame(GFxHandle handle, uint32_t frame);
    float GetMovieProgress(GFxHandle handle) const;

    // Renderização
    void RenderMovie(GFxHandle handle, const glm::mat4& projection, const glm::mat4& view);
    void RenderAllMovies(const glm::mat4& projection, const glm::mat4& view);
    void UpdateAllMovies(float deltaTime);

    // Interação com ActionScript
    bool InvokeFunction(GFxHandle handle, const std::string& functionName, 
                       const std::vector<std::string>& arguments = {});
    bool SetVariable(GFxHandle handle, const std::string& variableName, 
                    const std::string& variableValue);
    std::string GetVariable(GFxHandle handle, const std::string& variableName) const;

    // Callbacks e eventos
    typedef void (*GFxCallbackFunction)(const GFxCallbackEvent& event);
    void RegisterCallback(GFxHandle handle, const std::string& functionName, 
                         GFxCallbackFunction callback);
    void UnregisterCallback(GFxHandle handle, const std::string& functionName);
    void FireCallback(const GFxCallbackEvent& event);

    // Entrada (mouse/teclado)
    void OnMouseMove(float x, float y);
    void OnMouseClick(int button, bool pressed);
    void OnKeyPress(int keyCode, bool pressed);

    // Recursos e cache
    uint64_t GetTotalMemoryUsage() const;
    uint32_t GetMovieCount() const { return m_LoadedMovies.size(); }
    void ClearResourceCache();
    void PrintDebugInfo() const;

    // Configuração
    struct Config
    {
        uint32_t MaxMemoryMB = 512;
        bool EnableCompression = true;
        bool EnableMipMapping = true;
        bool EnableHardwareAcceleration = true;
        uint32_t MaxCachedFrames = 16;
        bool VerboseLogging = false;
    };

    void SetConfig(const Config& config) { m_Config = config; }
    const Config& GetConfig() const { return m_Config; }

    virtual ~CScaleformSystem();

private:
    CScaleformSystem();

    static CScaleformSystem* m_pInstance;

    // Estrutura interna de filme
    struct MovieInstance
    {
        GFxHandle Handle;
        std::string FilePath;
        GFxMovieInfo Info;
        std::vector<uint8_t> FileData;
        std::vector<uint32_t> FrameCache;
        std::unordered_map<std::string, std::string> Variables;
        std::unordered_map<std::string, GFxCallbackFunction> Callbacks;
        uint32_t TextureHandle;
        float ElapsedTime;
        bool NeedsRedraw;
    };

    bool m_bInitialized;
    Config m_Config;
    uint32_t m_NextHandle;

    std::vector<GFxHandle> m_LoadedMovies;
    std::unordered_map<GFxHandle, std::unique_ptr<MovieInstance>> m_MovieInstances;

    // Métodos privados
    bool ParseGFxFile(const std::string& filePath, MovieInstance& instance);
    bool DecompressGFx(const std::vector<uint8_t>& compressed, std::vector<uint8_t>& decompressed);
    bool CreateMovieTexture(MovieInstance& instance);
    void UpdateMovieFrame(MovieInstance& instance, float deltaTime);
    void RenderMovieFrame(const MovieInstance& instance, const glm::mat4& projection, const glm::mat4& view);
    std::string GetResourcePath(const std::string& filename) const;
    bool ValidateGFxHeader(const GFxFileHeader& header) const;
};

// Macro de acesso global
#define gScaleformSystem (CScaleformSystem::Instance())

/**
 * @brief Gerenciador de recursos de UI Scaleform
 * @details Cache automático, pooling de objetos e preload
 */
class CScaleformResourceCache
{
public:
    static CScaleformResourceCache* Instance();

    // Preload de recursos comuns
    void PreloadCommonUIs();
    void PreloadCharacterUI();
    void PreloadInventoryUI();
    void PreloadCombatUI();
    void PreloadPartyUI();
    void PreloadMapUI();

    // Obter filme pré-carregado
    GFxHandle GetPreloadedMovie(const std::string& uiName);
    void ClearPreloadedMovies();

    // Cache stats
    void PrintCacheStats() const;

private:
    CScaleformResourceCache();
    static CScaleformResourceCache* m_pInstance;

    std::unordered_map<std::string, GFxHandle> m_PreloadedMovies;
};

#define gScaleformCache (CScaleformResourceCache::Instance())

#include "stdafx.h"
#include "CScaleformSystem.h"
#include <fstream>
#include <algorithm>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

CScaleformSystem* CScaleformSystem::m_pInstance = nullptr;
CScaleformResourceCache* CScaleformResourceCache::m_pInstance = nullptr;

CScaleformSystem::CScaleformSystem()
    : m_bInitialized(false), m_NextHandle(1000)
{
}

CScaleformSystem::~CScaleformSystem()
{
    Shutdown();
}

CScaleformSystem* CScaleformSystem::Instance()
{
    if (!m_pInstance)
        m_pInstance = new CScaleformSystem();
    return m_pInstance;
}

bool CScaleformSystem::Initialize()
{
    if (m_bInitialized)
        return true;

    if (m_Config.MaxMemoryMB < 128)
        m_Config.MaxMemoryMB = 128;

    m_bInitialized = true;
    return true;
}

void CScaleformSystem::Shutdown()
{
    if (!m_bInitialized)
        return;

    UnloadAllMovies();
    m_bInitialized = false;
}

GFxHandle CScaleformSystem::LoadMovie(const std::string& filePath, uint32_t targetWidth, uint32_t targetHeight)
{
    if (!m_bInitialized || filePath.empty())
        return 0;

    // Verificar se já carregado
    for (const auto& loaded : m_LoadedMovies)
    {
        if (m_MovieInstances[loaded]->FilePath == filePath)
            return loaded;
    }

    GFxHandle newHandle = m_NextHandle++;
    auto movieInstance = std::make_unique<MovieInstance>();

    movieInstance->Handle = newHandle;
    movieInstance->FilePath = filePath;
    movieInstance->Info.FileName = filePath;
    movieInstance->Info.LoadState = GFxLoadState::LOADING;
    movieInstance->Info.Resolution = glm::uvec2(targetWidth, targetHeight);
    movieInstance->ElapsedTime = 0.0f;
    movieInstance->NeedsRedraw = true;

    if (!ParseGFxFile(filePath, *movieInstance))
    {
        movieInstance->Info.LoadState = GFxLoadState::LOAD_ERROR;
        m_MovieInstances[newHandle] = std::move(movieInstance);
        return newHandle;
    }

    if (!CreateMovieTexture(*movieInstance))
    {
        movieInstance->Info.LoadState = GFxLoadState::LOAD_ERROR;
    }
    else
    {
        movieInstance->Info.LoadState = GFxLoadState::LOADED;
        m_LoadedMovies.push_back(newHandle);
    }

    m_MovieInstances[newHandle] = std::move(movieInstance);
    return newHandle;
}

bool CScaleformSystem::UnloadMovie(GFxHandle handle)
{
    auto it = m_MovieInstances.find(handle);
    if (it == m_MovieInstances.end())
        return false;

    if (it->second->TextureHandle != 0)
        glDeleteTextures(1, &it->second->TextureHandle);

    m_MovieInstances.erase(it);

    auto filmIt = std::find(m_LoadedMovies.begin(), m_LoadedMovies.end(), handle);
    if (filmIt != m_LoadedMovies.end())
        m_LoadedMovies.erase(filmIt);

    return true;
}

void CScaleformSystem::UnloadAllMovies()
{
    std::vector<GFxHandle> handles = m_LoadedMovies;
    for (GFxHandle handle : handles)
    {
        UnloadMovie(handle);
    }
}

bool CScaleformSystem::IsMovieLoaded(GFxHandle handle) const
{
    auto it = m_MovieInstances.find(handle);
    if (it == m_MovieInstances.end())
        return false;
    return it->second->Info.LoadState == GFxLoadState::LOADED;
}

GFxMovieInfo CScaleformSystem::GetMovieInfo(GFxHandle handle) const
{
    auto it = m_MovieInstances.find(handle);
    if (it == m_MovieInstances.end())
        return GFxMovieInfo{};
    return it->second->Info;
}

GFxHandle CScaleformSystem::FindMovieByName(const std::string& movieName) const
{
    for (const auto& loaded : m_LoadedMovies)
    {
        auto it = m_MovieInstances.find(loaded);
        if (it != m_MovieInstances.end())
        {
            if (it->second->FilePath.find(movieName) != std::string::npos)
                return loaded;
        }
    }
    return 0;
}

void CScaleformSystem::PlayMovie(GFxHandle handle)
{
    auto it = m_MovieInstances.find(handle);
    if (it != m_MovieInstances.end())
    {
        it->second->Info.IsPlaying = true;
        it->second->NeedsRedraw = true;
    }
}

void CScaleformSystem::PauseMovie(GFxHandle handle)
{
    auto it = m_MovieInstances.find(handle);
    if (it != m_MovieInstances.end())
        it->second->Info.IsPlaying = false;
}

void CScaleformSystem::StopMovie(GFxHandle handle)
{
    auto it = m_MovieInstances.find(handle);
    if (it != m_MovieInstances.end())
    {
        it->second->Info.IsPlaying = false;
        it->second->Info.CurrentFrame = 0;
        it->second->ElapsedTime = 0.0f;
        it->second->NeedsRedraw = true;
    }
}

void CScaleformSystem::SetMovieFrame(GFxHandle handle, uint32_t frame)
{
    auto it = m_MovieInstances.find(handle);
    if (it != m_MovieInstances.end())
    {
        if (frame < it->second->Info.Header.FrameCount)
        {
            it->second->Info.CurrentFrame = static_cast<float>(frame);
            it->second->NeedsRedraw = true;
        }
    }
}

float CScaleformSystem::GetMovieProgress(GFxHandle handle) const
{
    auto it = m_MovieInstances.find(handle);
    if (it == m_MovieInstances.end())
        return 0.0f;

    uint16_t totalFrames = it->second->Info.Header.FrameCount;
    if (totalFrames == 0)
        return 0.0f;

    return it->second->Info.CurrentFrame / static_cast<float>(totalFrames);
}

void CScaleformSystem::RenderMovie(GFxHandle handle, const glm::mat4& projection, const glm::mat4& view)
{
    auto it = m_MovieInstances.find(handle);
    if (it == m_MovieInstances.end() || !IsMovieLoaded(handle))
        return;

    RenderMovieFrame(*it->second, projection, view);
}

void CScaleformSystem::RenderAllMovies(const glm::mat4& projection, const glm::mat4& view)
{
    for (GFxHandle handle : m_LoadedMovies)
    {
        RenderMovie(handle, projection, view);
    }
}

void CScaleformSystem::UpdateAllMovies(float deltaTime)
{
    for (GFxHandle handle : m_LoadedMovies)
    {
        auto it = m_MovieInstances.find(handle);
        if (it != m_MovieInstances.end())
        {
            UpdateMovieFrame(*it->second, deltaTime);
        }
    }
}

bool CScaleformSystem::InvokeFunction(GFxHandle handle, const std::string& functionName,
                                      const std::vector<std::string>& arguments)
{
    auto it = m_MovieInstances.find(handle);
    if (it == m_MovieInstances.end() || !IsMovieLoaded(handle))
        return false;
    return true;
}

bool CScaleformSystem::SetVariable(GFxHandle handle, const std::string& variableName,
                                   const std::string& variableValue)
{
    auto it = m_MovieInstances.find(handle);
    if (it == m_MovieInstances.end())
        return false;

    it->second->Variables[variableName] = variableValue;
    return true;
}

std::string CScaleformSystem::GetVariable(GFxHandle handle, const std::string& variableName) const
{
    auto it = m_MovieInstances.find(handle);
    if (it == m_MovieInstances.end())
        return "";

    auto varIt = it->second->Variables.find(variableName);
    if (varIt != it->second->Variables.end())
        return varIt->second;

    return "";
}

void CScaleformSystem::RegisterCallback(GFxHandle handle, const std::string& functionName,
                                        GFxCallbackFunction callback)
{
    auto it = m_MovieInstances.find(handle);
    if (it == m_MovieInstances.end())
        return;

    it->second->Callbacks[functionName] = callback;
}

void CScaleformSystem::UnregisterCallback(GFxHandle handle, const std::string& functionName)
{
    auto it = m_MovieInstances.find(handle);
    if (it == m_MovieInstances.end())
        return;

    it->second->Callbacks.erase(functionName);
}

void CScaleformSystem::FireCallback(const GFxCallbackEvent& event)
{
    auto it = m_MovieInstances.find(event.MovieHandle);
    if (it == m_MovieInstances.end())
        return;

    auto cbIt = it->second->Callbacks.find(event.FunctionName);
    if (cbIt != it->second->Callbacks.end())
    {
        cbIt->second(event);
    }
}

void CScaleformSystem::OnMouseMove(float x, float y)
{
    for (auto& pair : m_MovieInstances)
    {
        pair.second->NeedsRedraw = true;
    }
}

void CScaleformSystem::OnMouseClick(int button, bool pressed)
{
}

void CScaleformSystem::OnKeyPress(int keyCode, bool pressed)
{
}

uint64_t CScaleformSystem::GetTotalMemoryUsage() const
{
    uint64_t total = 0;
    for (const auto& pair : m_MovieInstances)
    {
        total += pair.second->Info.MemoryUsage;
    }
    return total;
}

void CScaleformSystem::ClearResourceCache()
{
    UnloadAllMovies();
}

void CScaleformSystem::PrintDebugInfo() const
{
}

// Metodos privados

bool CScaleformSystem::ParseGFxFile(const std::string& filePath, MovieInstance& instance)
{
    std::string fullPath = GetResourcePath(filePath);
    std::ifstream file(fullPath, std::ios::binary);

    if (!file.is_open())
        return false;

    GFxFileHeader header;
    file.read(header.Signature, 3);
    file.read(reinterpret_cast<char*>(&header.Version), 1);
    file.read(reinterpret_cast<char*>(&header.FrameSize), 4);
    file.read(reinterpret_cast<char*>(&header.FrameRate), 2);
    file.read(reinterpret_cast<char*>(&header.FrameCount), 2);

    if (!ValidateGFxHeader(header))
        return false;

    instance.Info.Header = header;

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    instance.FileData.resize(fileSize);
    file.read(reinterpret_cast<char*>(instance.FileData.data()), fileSize);
    file.close();

    instance.Info.MemoryUsage = fileSize;

    return true;
}

bool CScaleformSystem::DecompressGFx(const std::vector<uint8_t>& compressed, std::vector<uint8_t>& decompressed)
{
    decompressed = compressed;
    return true;
}

bool CScaleformSystem::CreateMovieTexture(MovieInstance& instance)
{
    uint32_t width = instance.Info.Resolution.x;
    uint32_t height = instance.Info.Resolution.y;

    glGenTextures(1, &instance.TextureHandle);
    glBindTexture(GL_TEXTURE_2D, instance.TextureHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    return instance.TextureHandle != 0;
}

void CScaleformSystem::UpdateMovieFrame(MovieInstance& instance, float deltaTime)
{
    if (!instance.Info.IsPlaying)
        return;

    float frameRate = instance.Info.Header.FrameRate / 256.0f;
    if (frameRate <= 0)
        frameRate = 24.0f;

    instance.ElapsedTime += deltaTime;
    float frameAdvance = (frameRate * deltaTime);
    instance.Info.CurrentFrame += frameAdvance;

    uint16_t frameCount = instance.Info.Header.FrameCount;
    if (instance.Info.CurrentFrame >= frameCount)
    {
        instance.Info.CurrentFrame = 0;
    }

    instance.NeedsRedraw = true;
}

void CScaleformSystem::RenderMovieFrame(const MovieInstance& instance, const glm::mat4& projection, const glm::mat4& view)
{
    if (instance.TextureHandle == 0)
        return;

    glBindTexture(GL_TEXTURE_2D, instance.TextureHandle);
}

std::string CScaleformSystem::GetResourcePath(const std::string& filename) const
{
    std::vector<std::string> searchPaths = {
        "Data/" + filename,
        filename,
        "UI/" + filename,
        "Interface/" + filename
    };

    for (const auto& path : searchPaths)
    {
        std::ifstream f(path);
        if (f.good())
            return path;
    }

    return "Data/" + filename;
}

bool CScaleformSystem::ValidateGFxHeader(const GFxFileHeader& header) const
{
    return header.IsValid();
}

// ==================== CScaleformResourceCache ====================

CScaleformResourceCache::CScaleformResourceCache()
{
}

CScaleformResourceCache* CScaleformResourceCache::Instance()
{
    if (!m_pInstance)
        m_pInstance = new CScaleformResourceCache();
    return m_pInstance;
}

void CScaleformResourceCache::PreloadCommonUIs()
{
    PreloadCharacterUI();
    PreloadInventoryUI();
    PreloadCombatUI();
    PreloadPartyUI();
    PreloadMapUI();
}

void CScaleformResourceCache::PreloadCharacterUI()
{
    auto handle = gScaleformSystem->LoadMovie("UI/Character.gfx", 1024, 768);
    if (handle)
        m_PreloadedMovies["character_ui"] = handle;
}

void CScaleformResourceCache::PreloadInventoryUI()
{
    auto handle = gScaleformSystem->LoadMovie("UI/Inventory.gfx", 800, 600);
    if (handle)
        m_PreloadedMovies["inventory_ui"] = handle;
}

void CScaleformResourceCache::PreloadCombatUI()
{
    auto handle = gScaleformSystem->LoadMovie("UI/Combat.gfx", 1280, 720);
    if (handle)
        m_PreloadedMovies["combat_ui"] = handle;
}

void CScaleformResourceCache::PreloadPartyUI()
{
    auto handle = gScaleformSystem->LoadMovie("UI/Party.gfx", 400, 300);
    if (handle)
        m_PreloadedMovies["party_ui"] = handle;
}

void CScaleformResourceCache::PreloadMapUI()
{
    auto handle = gScaleformSystem->LoadMovie("UI/Map.gfx", 512, 512);
    if (handle)
        m_PreloadedMovies["map_ui"] = handle;
}

GFxHandle CScaleformResourceCache::GetPreloadedMovie(const std::string& uiName)
{
    auto it = m_PreloadedMovies.find(uiName);
    if (it != m_PreloadedMovies.end())
        return it->second;
    return 0;
}

void CScaleformResourceCache::ClearPreloadedMovies()
{
    for (const auto& pair : m_PreloadedMovies)
    {
        gScaleformSystem->UnloadMovie(pair.second);
    }
    m_PreloadedMovies.clear();
}

void CScaleformResourceCache::PrintCacheStats() const
{
}

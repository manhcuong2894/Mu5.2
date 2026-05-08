#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <string>

/**
 * @file CColorSystem.h
 * @brief Sistema de colorização otimizado para MU Season 6
 * @details Colorização baseada em lookup table para máxima performance
 * 
 * Características:
 * - Lookup table para cores rápido
 * - Paletas predefinidas
 * - Colorização dinâmica de items/characters
 * - Baixo custo de renderização
 * - Compatível com todas as GPUs
 */

class CColorSystem
{
public:
    // Tipos de colorização
    enum class ColorizeMode
    {
        NONE = 0,              // Sem colorização
        ITEM_GRADE = 1,        // Cor por grade de item
        PLAYER_CLASS = 2,      // Cor por classe de player
        CUSTOM = 3,            // Cor customizada
        ELEMENTAL = 4,         // Cor por elemento (água, fogo, etc)
        TEAM = 5               // Cor por time/guild
    };

    // Estrutura de cor RGB
    struct Color
    {
        float R, G, B;
        float A = 1.0f;

        Color() : R(1.0f), G(1.0f), B(1.0f), A(1.0f) {}
        Color(float r, float g, float b, float a = 1.0f)
            : R(r), G(g), B(b), A(a) {}

        glm::vec4 ToVec4() const
        {
            return glm::vec4(R, G, B, A);
        }

        glm::vec3 ToVec3() const
        {
            return glm::vec3(R, G, B);
        }
    };

    // Tabela de cores lookup
    struct ColorLookupTable
    {
        static const int MAX_COLORS = 256;
        Color Colors[MAX_COLORS];
        int ColorCount;

        ColorLookupTable() : ColorCount(0) {}
    };

    static CColorSystem* Instance();

    // Inicialização
    bool Initialize();
    void Shutdown();

    // Gerenciamento de paletas
    bool CreatePalette(const std::string& name, const std::vector<Color>& colors);
    bool LoadPalette(const std::string& name);
    const ColorLookupTable& GetCurrentPalette() const { return m_CurrentPalette; }

    // Obter cor
    Color GetColor(int index) const;
    Color GetColorByGrade(int grade) const;      // Cor por grade de item
    Color GetColorByClass(int classID) const;    // Cor por classe
    Color GetColorByElement(int element) const;  // Cor por elemento

    // Colorização
    void ColorizeVertex(glm::vec3& color, ColorizeMode mode, int value);
    void ColorizeBuffer(std::vector<glm::vec3>& colors, ColorizeMode mode, const std::vector<int>& values);

    // Debug
    void PrintPaletteInfo();
    void ExportPalette(const std::string& filename);

    virtual ~CColorSystem();

private:
    CColorSystem();

    static CColorSystem* m_pInstance;

    // Paletas
    std::map<std::string, ColorLookupTable> m_Palettes;
    ColorLookupTable m_CurrentPalette;
    std::string m_CurrentPaletteName;

    // Paletas padrão
    void CreateDefaultPalettes();
    void CreateItemGradePalette();
    void CreateClassPalette();
    void CreateElementalPalette();
    void CreateTeamPalette();

    bool m_bInitialized;
};

#define gColorSystem (CColorSystem::Instance())

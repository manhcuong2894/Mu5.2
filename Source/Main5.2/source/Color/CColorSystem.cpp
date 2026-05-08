#include "stdafx.h"
#include "CColorSystem.h"
#include <iostream>
#include <fstream>

CColorSystem* CColorSystem::m_pInstance = nullptr;

CColorSystem::CColorSystem()
    : m_CurrentPaletteName("default")
    , m_bInitialized(false)
{
}

CColorSystem::~CColorSystem()
{
    Shutdown();
}

CColorSystem* CColorSystem::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CColorSystem();
    }
    return m_pInstance;
}

bool CColorSystem::Initialize()
{
    if (m_bInitialized)
        return true;

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Color System Inicialização" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Criar paletas padrão
    CreateDefaultPalettes();

    // Carregar paleta padrão
    if (!LoadPalette("item_grade"))
    {
        std::cerr << "[Color System] Falha ao carregar paleta padrão!" << std::endl;
        return false;
    }

    m_bInitialized = true;

    std::cout << "[Color System] ? Inicializado!" << std::endl;
    std::cout << "[Color System] Paletas carregadas: " << m_Palettes.size() << std::endl;

    return true;
}

void CColorSystem::Shutdown()
{
    if (!m_bInitialized)
        return;

    m_Palettes.clear();
    m_bInitialized = false;

    std::cout << "[Color System] Finalizado!" << std::endl;
}

bool CColorSystem::CreatePalette(const std::string& name, const std::vector<Color>& colors)
{
    if (colors.empty() || colors.size() > ColorLookupTable::MAX_COLORS)
    {
        std::cerr << "[Color System] Tamanho de paleta inválido!" << std::endl;
        return false;
    }

    ColorLookupTable lut;
    lut.ColorCount = colors.size();

    for (size_t i = 0; i < colors.size(); ++i)
    {
        lut.Colors[i] = colors[i];
    }

    m_Palettes[name] = lut;

    std::cout << "[Color System] Paleta '" << name << "' criada com " << colors.size() << " cores" << std::endl;

    return true;
}

bool CColorSystem::LoadPalette(const std::string& name)
{
    auto it = m_Palettes.find(name);
    if (it == m_Palettes.end())
    {
        std::cerr << "[Color System] Paleta '" << name << "' não encontrada!" << std::endl;
        return false;
    }

    m_CurrentPalette = it->second;
    m_CurrentPaletteName = name;

    std::cout << "[Color System] Paleta '" << name << "' carregada (" << m_CurrentPalette.ColorCount << " cores)" << std::endl;

    return true;
}

CColorSystem::Color CColorSystem::GetColor(int index) const
{
    if (index < 0 || index >= m_CurrentPalette.ColorCount)
    {
        return Color(1.0f, 1.0f, 1.0f);  // Branco como fallback
    }

    return m_CurrentPalette.Colors[index];
}

CColorSystem::Color CColorSystem::GetColorByGrade(int grade) const
{
    // Grade normal (0-10) mapeado para 0-10 índice
    int index = grade % m_CurrentPalette.ColorCount;
    return GetColor(index);
}

CColorSystem::Color CColorSystem::GetColorByClass(int classID) const
{
    // Classe mapeado para cor
    // 0=Dark Lord, 1=Soul Master, 2=Elf, 3=Magic Gladiator, 4=Dark Knight
    int index = classID % m_CurrentPalette.ColorCount;
    return GetColor(index);
}

CColorSystem::Color CColorSystem::GetColorByElement(int element) const
{
    // Elemento: 0=None, 1=Water, 2=Fire, 3=Earth, 4=Wind, 5=Dark, 6=Holy
    int index = element % m_CurrentPalette.ColorCount;
    return GetColor(index);
}

void CColorSystem::ColorizeVertex(glm::vec3& color, ColorizeMode mode, int value)
{
    Color newColor;

    switch (mode)
    {
    case ColorizeMode::ITEM_GRADE:
        newColor = GetColorByGrade(value);
        break;
    case ColorizeMode::PLAYER_CLASS:
        newColor = GetColorByClass(value);
        break;
    case ColorizeMode::ELEMENTAL:
        newColor = GetColorByElement(value);
        break;
    case ColorizeMode::CUSTOM:
        newColor = GetColor(value);
        break;
    case ColorizeMode::NONE:
    default:
        return;
    }

    color = newColor.ToVec3();
}

void CColorSystem::ColorizeBuffer(std::vector<glm::vec3>& colors, ColorizeMode mode, const std::vector<int>& values)
{
    if (colors.size() != values.size())
    {
        std::cerr << "[Color System] Tamanho de buffer incompatível!" << std::endl;
        return;
    }

    for (size_t i = 0; i < colors.size(); ++i)
    {
        ColorizeVertex(colors[i], mode, values[i]);
    }
}

void CColorSystem::PrintPaletteInfo()
{
    std::cout << "\n[Color System] Paletas disponíveis:" << std::endl;

    for (const auto& pair : m_Palettes)
    {
        std::cout << "  - " << pair.first << " (" << pair.second.ColorCount << " cores)";
        if (pair.first == m_CurrentPaletteName)
        {
            std::cout << " [ATIVA]";
        }
        std::cout << std::endl;
    }
}

void CColorSystem::ExportPalette(const std::string& filename)
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "[Color System] Falha ao abrir arquivo!" << std::endl;
        return;
    }

    file << "# Paleta: " << m_CurrentPaletteName << std::endl;
    file << "# Cores: " << m_CurrentPalette.ColorCount << std::endl;
    file << std::endl;

    for (int i = 0; i < m_CurrentPalette.ColorCount; ++i)
    {
        const Color& c = m_CurrentPalette.Colors[i];
        file << "Color " << i << ": " << c.R << " " << c.G << " " << c.B << " " << c.A << std::endl;
    }

    file.close();

    std::cout << "[Color System] Paleta exportada para '" << filename << "'" << std::endl;
}

void CColorSystem::CreateDefaultPalettes()
{
    // Paleta de grades de item
    std::vector<Color> itemGradePalette = {
        Color(1.0f, 1.0f, 1.0f),  // 0 - Normal (Branco)
        Color(0.0f, 1.0f, 0.0f),  // 1 - Comum (Verde)
        Color(0.0f, 0.5f, 1.0f),  // 2 - Raro (Azul)
        Color(1.0f, 0.0f, 1.0f),  // 3 - Épico (Roxo)
        Color(1.0f, 1.0f, 0.0f),  // 4 - Lendário (Amarelo)
        Color(1.0f, 0.5f, 0.0f),  // 5 - Héroi (Laranja)
        Color(0.8f, 0.2f, 0.2f),  // 6 - Antigo (Vermelho)
        Color(0.5f, 0.5f, 0.5f),  // 7 - Desconhecido (Cinza)
        Color(1.0f, 0.8f, 0.0f),  // 8 - Especial (Ouro)
        Color(0.2f, 0.8f, 1.0f),  // 9 - Místico (Ciano)
        Color(0.9f, 0.1f, 0.9f)   // 10 - Supremo (Rosa)
    };

    CreatePalette("item_grade", itemGradePalette);

    // Paleta de classes
    std::vector<Color> classPalette = {
        Color(0.3f, 0.0f, 0.0f),  // 0 - Dark Lord (Vermelho escuro)
        Color(0.0f, 0.0f, 0.8f),  // 1 - Soul Master (Azul)
        Color(0.2f, 0.8f, 0.2f),  // 2 - Elf (Verde claro)
        Color(1.0f, 0.7f, 0.0f),  // 3 - Magic Gladiator (Ouro)
        Color(0.5f, 0.2f, 0.7f),  // 4 - Dark Knight (Roxo)
        Color(0.8f, 0.4f, 0.0f),  // 5 - Kung Fu Master (Laranja)
        Color(0.8f, 0.8f, 0.0f)   // 6 - Lorde do Caos (Amarelo)
    };

    CreatePalette("player_class", classPalette);

    // Paleta de elementos
    std::vector<Color> elementalPalette = {
        Color(1.0f, 1.0f, 1.0f),  // 0 - None (Branco)
        Color(0.0f, 0.5f, 1.0f),  // 1 - Water (Azul)
        Color(1.0f, 0.0f, 0.0f),  // 2 - Fire (Vermelho)
        Color(0.8f, 0.6f, 0.2f),  // 3 - Earth (Marrom)
        Color(0.5f, 1.0f, 0.5f),  // 4 - Wind (Verde claro)
        Color(0.2f, 0.0f, 0.2f),  // 5 - Dark (Preto)
        Color(1.0f, 1.0f, 0.7f)   // 6 - Holy (Amarelo claro)
    };

    CreatePalette("elemental", elementalPalette);

    // Paleta de times/guilds
    std::vector<Color> teamPalette = {
        Color(1.0f, 0.0f, 0.0f),  // 0 - Time 1 (Vermelho)
        Color(0.0f, 0.0f, 1.0f),  // 1 - Time 2 (Azul)
        Color(0.0f, 1.0f, 0.0f),  // 2 - Time 3 (Verde)
        Color(1.0f, 1.0f, 0.0f),  // 3 - Time 4 (Amarelo)
        Color(1.0f, 0.0f, 1.0f),  // 4 - Time 5 (Magenta)
        Color(0.0f, 1.0f, 1.0f),  // 5 - Time 6 (Ciano)
        Color(1.0f, 0.5f, 0.0f)   // 6 - Team 7 (Laranja)
    };

    CreatePalette("team", teamPalette);

    std::cout << "[Color System] Paletas padrão criadas" << std::endl;
}

void CColorSystem::CreateItemGradePalette()
{
    // Já criada em CreateDefaultPalettes
}

void CColorSystem::CreateClassPalette()
{
    // Já criada em CreateDefaultPalettes
}

void CColorSystem::CreateElementalPalette()
{
    // Já criada em CreateDefaultPalettes
}

void CColorSystem::CreateTeamPalette()
{
    // Já criada em CreateDefaultPalettes
}

#pragma once

/**
 * @file MUColorizeExamples.cpp
 * @brief Exemplos práticos de uso do sistema de cores em MU Season 6
 */

#include "MUColorize.h"

// ============================================================
// EXEMPLO 1: Renderizar personagens com cores de classe
// ============================================================

void RenderPlayersWithClassColors()
{
    // Suponha que você tenha estrutura de player
    struct PlayerData
    {
        GLuint modelVAO;
        GLuint modelShader;
        int modelIndexCount;
        MU::CharacterClass classID;
    };

    std::vector<PlayerData> players = {
        // { vao1, shader, count, DarkLord },
        // { vao2, shader, count, SoulMaster },
        // ...
    };

    // Renderizar cada player com cor de classe
    for (const auto& player : players)
    {
        MU::RenderCharacterWithClass(
            player.modelVAO,
            player.modelShader,
            player.modelIndexCount,
            player.classID
        );
    }
}

// ============================================================
// EXEMPLO 2: Renderizar itens com cores de grade
// ============================================================

void RenderItemsWithGradeColors()
{
    struct ItemData
    {
        GLuint modelVAO;
        GLuint modelShader;
        int modelIndexCount;
        MU::ItemGrade grade;
    };

    std::vector<ItemData> items = {
        // { vao1, shader, count, Normal },
        // { vao2, shader, count, Rare },
        // { vao3, shader, count, Epic },
        // ...
    };

    // Renderizar cada item com cor de grade
    for (const auto& item : items)
    {
        MU::RenderItemWithGrade(
            item.modelVAO,
            item.modelShader,
            item.modelIndexCount,
            item.grade
        );
    }
}

// ============================================================
// EXEMPLO 3: Renderizar efeitos com cores de elemento
// ============================================================

void RenderEffectsWithElementColors()
{
    struct EffectData
    {
        GLuint modelVAO;
        GLuint modelShader;
        int modelIndexCount;
        MU::ElementType element;
    };

    std::vector<EffectData> effects = {
        // { vao1, shader, count, Fire },
        // { vao2, shader, count, Water },
        // { vao3, shader, count, Holy },
        // ...
    };

    // Renderizar cada efeito com cor de elemento
    for (const auto& effect : effects)
    {
        MU::RenderEffectWithElement(
            effect.modelVAO,
            effect.modelShader,
            effect.modelIndexCount,
            effect.element
        );
    }
}

// ============================================================
// EXEMPLO 4: Batch rendering (MAIS RÁPIDO)
// ============================================================

void RenderPlayersBatchOptimized()
{
    struct PlayerData
    {
        GLuint modelVAO;
        GLuint modelShader;
        int modelIndexCount;
        MU::CharacterClass classID;
    };

    std::vector<PlayerData> players;
    // ... popular players ...

    // Preparar dados para batch
    std::vector<GLuint> vaos;
    std::vector<GLuint> shaders;
    std::vector<int> indexCounts;
    std::vector<MU::CharacterClass> classes;

    for (const auto& player : players)
    {
        vaos.push_back(player.modelVAO);
        shaders.push_back(player.modelShader);
        indexCounts.push_back(player.modelIndexCount);
        classes.push_back(player.classID);
    }

    // Renderizar todo o batch em uma chamada
    MU::RenderCharacterBatch(vaos, shaders, indexCounts, classes);
}

// ============================================================
// EXEMPLO 5: Obter cores para UI (minimap, nameplates, etc)
// ============================================================

void UpdatePlayerNametag(const std::string& playerName, MU::CharacterClass classID)
{
    // Obter cor da classe
    CColorSystem::Color classColor = MU::GetClassColor(classID);

    // Usar em UI (exemplo)
    // float r = classColor.R;
    // float g = classColor.G;
    // float b = classColor.B;
    // drawNametag(playerName, r, g, b);
}

void UpdateItemLootColor(MU::ItemGrade grade)
{
    // Obter cor da grade
    CColorSystem::Color gradeColor = MU::GetItemGradeColor(grade);

    // Usar em loot indicator
    // drawLootIndicator(gradeColor.R, gradeColor.G, gradeColor.B);
}

// ============================================================
// EXEMPLO 6: Renderizar cena completa com cores
// ============================================================

void RenderGameSceneWithColors()
{
    // Renderizar players
    RenderPlayersWithClassColors();

    // Renderizar itens
    RenderItemsWithGradeColors();

    // Renderizar efeitos
    RenderEffectsWithElementColors();

    // Tudo com cores automáticas, sem perda de FPS!
}

// ============================================================
// EXEMPLO 7: Inicialização no Winmain
// ============================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    // ... código existente do seu projeto ...

    // Inicializar sistema de cores MU
    if (!MU::InitializeColorSystem())
    {
        MessageBox(NULL, "Falha ao inicializar sistema de cores!", "Erro", MB_OK);
        // Continuar mesmo assim (renderização sem cores)
    }

    // ... resto do código ...

    // Na finalização:
    // MU::ShutdownColorSystem();
}

// ============================================================
// EXEMPLO 8: Renderizar com cores customizadas
// ============================================================

void RenderWithCustomPalette()
{
    // Criar paleta customizada
    std::vector<CColorSystem::Color> myColors = {
        CColorSystem::Color(1.0f, 0.0f, 0.0f),  // Vermelho
        CColorSystem::Color(0.0f, 1.0f, 0.0f),  // Verde
        CColorSystem::Color(0.0f, 0.0f, 1.0f),  // Azul
    };

    gColorSystem->CreatePalette("minha_paleta", myColors);
    gColorSystem->LoadPalette("minha_paleta");

    // Renderizar com paleta customizada
    MU::RenderWithCustomColor(vao, shader, indexCount, 0);  // Vermelho
    MU::RenderWithCustomColor(vao, shader, indexCount, 1);  // Verde
    MU::RenderWithCustomColor(vao, shader, indexCount, 2);  // Azul
}

// ============================================================
// EXEMPLO 9: Debug - Imprimir informações de cores
// ============================================================

void DebugColorSystem()
{
    // Ver paletas disponíveis
    gColorSystem->PrintPaletteInfo();

    // Ver estatísticas
    int drawCalls = gColorizedRenderer->GetLastDrawCallCount();
    int vertices = gColorizedRenderer->GetLastVertexCount();

    std::cout << "Draw Calls: " << drawCalls << std::endl;
    std::cout << "Vertices: " << vertices << std::endl;

    // Exportar paleta
    gColorSystem->ExportPalette("palette_backup.txt");
}

// ============================================================
// EXEMPLO 10: Integração com seu código MU existente
// ============================================================

// Se você tem função de renderização assim:
// void RenderCharacter(GLuint vao, GLuint shader, int count, CharacterData& data)
// {
//     glBindVertexArray(vao);
//     glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
// }

// Mude para:
void RenderCharacterWithColor(GLuint vao, GLuint shader, int count, MU::CharacterClass classID)
{
    // Renderiza E aplica cor da classe
    MU::RenderCharacterWithClass(vao, shader, count, classID);
}

// Mesmo assim para itens:
void RenderItemWithColor(GLuint vao, GLuint shader, int count, MU::ItemGrade grade)
{
    // Renderiza E aplica cor da grade
    MU::RenderItemWithGrade(vao, shader, count, grade);
}

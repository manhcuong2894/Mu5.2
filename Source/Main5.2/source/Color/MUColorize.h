#pragma once

/**
 * @file MUColorize.h
 * @brief Funções de conveniência para colorização em MU Season 6
 * @details Wrappers simples para usar cores em seu projeto
 */

#include "CColorSystem.h"
#include "CColorizedRenderer.h"
#include <GL/glew.h>

// Namespace para MU
namespace MU
{
    // Classes de personagem
    enum class CharacterClass
    {
        DarkLord = 0,
        SoulMaster = 1,
        Elf = 2,
        MagicGladiator = 3,
        DarkKnight = 4,
        KungFuMaster = 5,
        ChaosLord = 6
    };

    // Grades de item
    enum class ItemGrade
    {
        Normal = 0,
        Common = 1,
        Rare = 2,
        Epic = 3,
        Legendary = 4,
        Hero = 5,
        Ancient = 6,
        Unknown = 7,
        Special = 8,
        Mystic = 9,
        Supreme = 10
    };

    // Tipos de elemento
    enum class ElementType
    {
        None = 0,
        Water = 1,
        Fire = 2,
        Earth = 3,
        Wind = 4,
        Dark = 5,
        Holy = 6
    };

    // Renderizar personagem com cor de classe
    inline void RenderCharacterWithClass(
        GLuint vao,
        GLuint shader,
        int indexCount,
        CharacterClass classID)
    {
        gColorizedRenderer->RenderColorized(
            vao, shader, indexCount,
            CColorSystem::ColorizeMode::PLAYER_CLASS,
            static_cast<int>(classID)
        );
    }

    // Renderizar item com cor de grade
    inline void RenderItemWithGrade(
        GLuint vao,
        GLuint shader,
        int indexCount,
        ItemGrade grade)
    {
        gColorizedRenderer->RenderColorized(
            vao, shader, indexCount,
            CColorSystem::ColorizeMode::ITEM_GRADE,
            static_cast<int>(grade)
        );
    }

    // Renderizar efeito com cor de elemento
    inline void RenderEffectWithElement(
        GLuint vao,
        GLuint shader,
        int indexCount,
        ElementType element)
    {
        gColorizedRenderer->RenderColorized(
            vao, shader, indexCount,
            CColorSystem::ColorizeMode::ELEMENTAL,
            static_cast<int>(element)
        );
    }

    // Renderizar com team color
    inline void RenderWithTeamColor(
        GLuint vao,
        GLuint shader,
        int indexCount,
        int teamID)
    {
        gColorizedRenderer->RenderColorized(
            vao, shader, indexCount,
            CColorSystem::ColorizeMode::TEAM,
            teamID % 7
        );
    }

    // Renderizar com cor customizada
    inline void RenderWithCustomColor(
        GLuint vao,
        GLuint shader,
        int indexCount,
        int colorIndex)
    {
        gColorizedRenderer->RenderColorized(
            vao, shader, indexCount,
            CColorSystem::ColorizeMode::CUSTOM,
            colorIndex
        );
    }

    // Renderizar múltiplos personagens com cores
    inline void RenderCharacterBatch(
        const std::vector<GLuint>& vaos,
        const std::vector<GLuint>& shaders,
        const std::vector<int>& indexCounts,
        const std::vector<CharacterClass>& classes)
    {
        std::vector<CColorSystem::ColorizeMode> modes(vaos.size(),
            CColorSystem::ColorizeMode::PLAYER_CLASS);
        std::vector<int> classValues;

        for (const auto& c : classes)
        {
            classValues.push_back(static_cast<int>(c));
        }

        gColorizedRenderer->RenderBatch(vaos, shaders, indexCounts, modes, classValues);
    }

    // Renderizar múltiplos itens com cores
    inline void RenderItemBatch(
        const std::vector<GLuint>& vaos,
        const std::vector<GLuint>& shaders,
        const std::vector<int>& indexCounts,
        const std::vector<ItemGrade>& grades)
    {
        std::vector<CColorSystem::ColorizeMode> modes(vaos.size(),
            CColorSystem::ColorizeMode::ITEM_GRADE);
        std::vector<int> gradeValues;

        for (const auto& g : grades)
        {
            gradeValues.push_back(static_cast<int>(g));
        }

        gColorizedRenderer->RenderBatch(vaos, shaders, indexCounts, modes, gradeValues);
    }

    // Obter cor de classe
    inline CColorSystem::Color GetClassColor(CharacterClass classID)
    {
        return gColorSystem->GetColorByClass(static_cast<int>(classID));
    }

    // Obter cor de grade
    inline CColorSystem::Color GetItemGradeColor(ItemGrade grade)
    {
        return gColorSystem->GetColorByGrade(static_cast<int>(grade));
    }

    // Obter cor de elemento
    inline CColorSystem::Color GetElementColor(ElementType element)
    {
        return gColorSystem->GetColorByElement(static_cast<int>(element));
    }

    // Inicializar sistema de cores
    inline bool InitializeColorSystem()
    {
        if (!gColorSystem->Initialize())
            return false;

        if (!gColorizedRenderer->Initialize())
            return false;

        return true;
    }

    // Shutdown
    inline void ShutdownColorSystem()
    {
        gColorizedRenderer->Shutdown();
        // gColorSystem->Shutdown(); // Singleton, não precisa
    }
}

#define MU_COLORIZE_READY (gColorSystem && gColorizedRenderer)

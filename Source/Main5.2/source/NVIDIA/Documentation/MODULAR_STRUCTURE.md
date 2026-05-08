# ??? Estrutura Modular NVIDIA - Guia Completo

## ?? Visão Geral

O sistema NVIDIA foi reorganizado em uma **arquitetura modular** para facilitar manutenção, testes e futuras expansões.

---

## ?? Estrutura de Diretórios

```
source/NVIDIA/
?
??? Core/                          ? Núcleo do sistema (obrigatório)
?   ??? CNvidiaGPUManager.h/cpp
?   ??? CNvidiaRenderSystem.h/cpp
?   ??? CNvidiaMemoryManager.h/cpp
?   ??? CNvidiaSystem.h/cpp
?   ??? CNvidiaDebugMonitor.h/cpp
?   ??? (classes principais)
?
??? Modules/                       ? Módulos específicos (expansível)
?   ?
?   ??? Terrain/                   ? Renderização de terreno
?   ?   ??? CTerrainNvidia.h       ? Classe principal
?   ?   ??? CTerrainNvidia.cpp     ? Implementação
?   ?   ??? TerrainNvidiaConfig.h  ? Configurações
?   ?   ??? terrain.nvidia.vs      ? Vertex shader
?   ?   ??? terrain.nvidia.fs      ? Fragment shader
?   ?   ??? TERRAIN_GUIDE.md       ? Documentação
?   ?
?   ??? Character/                 ? Renderização de personagens (futuro)
?   ?   ??? CCharacterNvidia.h
?   ?   ??? CCharacterNvidia.cpp
?   ?   ??? CharacterNvidiaConfig.h
?   ?   ??? character.nvidia.vs
?   ?   ??? character.nvidia.fs
?   ?   ??? CHARACTER_GUIDE.md
?   ?
?   ??? Effects/                   ? Efeitos especiais (futuro)
?   ?   ??? CEffectsNvidia.h
?   ?   ??? CEffectsNvidia.cpp
?   ?   ??? EffectsNvidiaConfig.h
?   ?   ??? effects.nvidia.vs
?   ?   ??? effects.nvidia.fs
?   ?   ??? EFFECTS_GUIDE.md
?   ?
?   ??? [Adicionar mais módulos conforme necessário]
?
??? Documentation/                 ? Documentação
?   ??? MODULAR_STRUCTURE.md       ? Este arquivo
?   ??? HOW_TO_ADD_MODULE.md       ? Como adicionar novo módulo
?   ??? TERRAIN_NVIDIA_GUIDE.md    ? Guia específico do terrain
?   ??? ARCHITECTURE.md            ? Arquitetura geral
?
??? [Arquivos gerais do NVIDIA]
    ??? README.md
    ??? INTEGRATION_GUIDE.md
    ??? ...
```

---

## ?? Princípios de Design

### 1. **Modularidade**
Cada tipo de renderização tem seu próprio módulo:
- Código isolado
- Configuração independente
- Fácil de remover/adicionar

### 2. **Configuração por Perfil**
Cada módulo tem 3 perfis de configuração:
- `DEFAULT_CONFIG` - Padrão balanceado
- `PERFORMANCE_CONFIG` - Otimizado para FPS
- `QUALITY_CONFIG` - Máxima qualidade

### 3. **Shader Específico**
Cada tipo de renderização tem shaders otimizados:
- `.nvidia.vs` - Vertex shader
- `.nvidia.fs` - Fragment shader

### 4. **Singleton Pattern**
Cada módulo usa singleton para acesso global:
```cpp
gTerrainNvidia->Render(...);  // Em vez de criar instâncias
```

---

## ?? Como Usar Cada Módulo

### Terrain NVIDIA

```cpp
#include "NVIDIA/Modules/Terrain/CTerrainNvidia.h"

// Inicializar
gTerrainNvidia->Initialize(TerrainNvidia::DEFAULT_CONFIG);

// Adicionar chunk de terreno
gTerrainNvidia->AddChunk(0, 0, 1024.0f, terrainData);

// Em cada frame
gTerrainNvidia->Update(cameraPosition);
gTerrainNvidia->Render(viewMatrix, projMatrix);

// Alterar configuração
TerrainNvidia::Config config = TerrainNvidia::QUALITY_CONFIG;
gTerrainNvidia->SetConfig(config);
```

---

## ?? Padrão de Configuração

Cada módulo segue este padrão:

```cpp
// ModuleConfig.h
namespace ModuleNvidia
{
    struct Config
    {
        // Renderização
        bool EnableDeferred;
        bool EnableInstancing;
        
        // Qualidade
        float DetailLevel;
        
        // Performance
        bool EnableGPUCulling;
    };

    // Perfis pré-configurados
    static const Config DEFAULT_CONFIG = { ... };
    static const Config PERFORMANCE_CONFIG = { ... };
    static const Config QUALITY_CONFIG = { ... };
}
```

---

## ?? Como Adicionar um Novo Módulo

### Passo 1: Criar estrutura de pastas

```
source/NVIDIA/Modules/[NomeDoModulo]/
??? C[NomeDoModulo]Nvidia.h
??? C[NomeDoModulo]Nvidia.cpp
??? [NomeDoModulo]NvidiaConfig.h
??? [nomedo modulo].nvidia.vs
??? [nomedomodulo].nvidia.fs
??? [NOMEDOMODULO]_GUIDE.md
```

### Passo 2: Criar classe configuration

```cpp
// MyModuleNvidiaConfig.h
namespace MyModuleNvidia
{
    struct Config
    {
        bool EnableFeatureX;
        bool EnableFeatureY;
        float Parameter1;
        // ...
    };

    static const Config DEFAULT_CONFIG = { ... };
    static const Config PERFORMANCE_CONFIG = { ... };
    static const Config QUALITY_CONFIG = { ... };
}
```

### Passo 3: Criar classe principal

```cpp
// CMyModuleNvidia.h
class CMyModuleNvidia
{
public:
    static CMyModuleNvidia* Instance();
    
    bool Initialize(const MyModuleNvidia::Config& config);
    void Shutdown();
    
    void Update(const glm::vec3& cameraPosition);
    void Render(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
    
    // ... métodos específicos ...

private:
    static CMyModuleNvidia* m_pInstance;
    MyModuleNvidia::Config m_Config;
};

#define gMyModuleNvidia (CMyModuleNvidia::Instance())
```

### Passo 4: Criar shaders otimizados

```glsl
// mymodule.nvidia.vs
#version 450 core
// Vertex shader otimizado para NVIDIA
```

### Passo 5: Documentar

Criar `MYMODULE_GUIDE.md` com:
- Como usar
- Exemplos de código
- Configurações disponíveis
- Performance tips

---

## ?? Ordem de Implementação Sugerida

1. ? **Terrain** (em desenvolvimento)
2. ? **Character** (próximo)
3. ? **Effects** (depois)
4. ? **Water** (futuro)
5. ? **Vegetation** (futuro)

---

## ?? Integração com Winmain.cpp

Exemplo de como integrar todos os módulos:

```cpp
// Winmain.cpp

#include "NVIDIA/Modules/Terrain/CTerrainNvidia.h"
#include "NVIDIA/Modules/Character/CCharacterNvidia.h"  // Futuro
#include "NVIDIA/Modules/Effects/CEffectsNvidia.h"      // Futuro

bool InitializeNVIDIAModules()
{
    // Inicializar módulos
    if (!gTerrainNvidia->Initialize(TerrainNvidia::DEFAULT_CONFIG))
        return false;

    // if (!gCharacterNvidia->Initialize(CharacterNvidia::DEFAULT_CONFIG))
    //     return false;

    // if (!gEffectsNvidia->Initialize(EffectsNvidia::DEFAULT_CONFIG))
    //     return false;

    return true;
}

void RenderNVIDIAModules(const glm::mat4& view, const glm::mat4& proj, 
                         const glm::vec3& cameraPos)
{
    // Renderizar terreno
    gTerrainNvidia->Update(cameraPos);
    gTerrainNvidia->Render(view, proj);

    // Renderizar characters (futuro)
    // gCharacterNvidia->Update(cameraPos);
    // gCharacterNvidia->Render(view, proj);

    // Renderizar efeitos (futuro)
    // gEffectsNvidia->Update(cameraPos);
    // gEffectsNvidia->Render(view, proj);
}

void ShutdownNVIDIAModules()
{
    gTerrainNvidia->Shutdown();
    // gCharacterNvidia->Shutdown();
    // gEffectsNvidia->Shutdown();
}
```

---

## ?? Checklist para Novo Módulo

- [ ] Pasta criada em `Modules/[NomeDoModulo]/`
- [ ] Arquivo `Config.h` com 3 perfis
- [ ] Classe principal `C[Nome]Nvidia.h/cpp`
- [ ] Shaders `.vs` e `.fs` otimizados
- [ ] Documentação `[NOME]_GUIDE.md`
- [ ] Compilação sem erros
- [ ] Integração em Winmain.cpp
- [ ] Testes de performance

---

## ?? Exemplo: Terrain é o Padrão

O **Terrain** foi criado como exemplo perfeito:

? Tem `TerrainNvidiaConfig.h` com 3 perfis
? Tem `CTerrainNvidia.h/cpp` completo
? Tem `terrain.nvidia.vs/fs` otimizados
? Tem singleton `gTerrainNvidia`
? Segue o padrão de módulo

**Use como referência para adicionar novos módulos!**

---

## ?? Próximos Passos

1. **Terrain está pronto** - Integre em seu código
2. **Character será similar** - Copie o padrão do Terrain
3. **Effects será modular** - Extensível para vários tipos
4. **Mantenha a modularidade** - Cada renderização isolada

---

## ?? Dicas

- **Não misture módulos** - Cada um é independente
- **Use os configs** - 3 perfis para cada caso
- **Documente bem** - Facilita futuras modificações
- **Teste isolado** - Teste cada módulo separado
- **Reutilize padrões** - Copy/paste do Terrain para novo módulo

---

**Versão**: 1.0
**Data**: 2024
**Status**: Estrutura pronta para expansão

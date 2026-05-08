# ?? Sistema AMD - Guia Completo

## ?? Visão Geral

O sistema **AMD Render** oferece renderização otimizada para GPUs AMD com:

? Renderização Deferred otimizada
? Wave Optimization (Wave64)
? Local Data Store (LDS) para otimizações
? Predicate Execution
? Light Culling eficiente
? Suporte a múltiplas luzes (até 64)

---

## ?? Características AMD

### Renderização Deferred
```
Renderizar geometria para G-Buffers
    ?
Position, Normal, Albedo, etc
    ?
Light pass com múltiplas luzes
    ?
Composição final
```

### Otimizações Wave (RDNA)
- **Wave64**: Execução de 64 threads em paralelo
- **Predicates**: Execução condicional eficiente
- **LDS**: Memória compartilhada para cache de dados

### Light Culling
- Compute shader para cull lights
- Grid 3D de luzes
- Acesso eficiente na fragment shader

---

## ?? Quick Start

### 1. Inicializar AMD Render

```cpp
#include "AMD/Core/CAMDRenderSystem.h"

// Com configuração padrão
gAMDRender->Initialize(CAMDRenderSystem::GetDefaultConfig());

// Com configuração customizada
CAMDRenderSystem::RenderConfig config;
config.EnableDeferred = true;
config.MaxLights = 32;
config.DetailLevel = 0.8f;
gAMDRender->Initialize(config);
```

### 2. Usar em Loop de Renderização

```cpp
void MainScene(HDC hDC)
{
    gAMDRender->BeginFrame();
    gAMDRender->BeginDeferredPass();
    
    // Renderizar geometria
    // ...
    
    gAMDRender->RenderLightPass();
    gAMDRender->EndFrame();
}
```

---

## ?? Configurações AMD

### RenderConfig

```cpp
struct RenderConfig
{
    // Renderização
    bool EnableDeferred = true;          // Deferred rendering
    bool EnableInstancing = true;        // Instancing de geometria
    
    // Otimizações AMD específicas
    bool EnableWaveOptimization = true;  // Wave64 optimization
    bool EnableLDSUsage = true;          // Local Data Store
    bool EnablePredicates = true;        // Predicate execution
    
    // Qualidade
    float DetailLevel = 0.8f;            // 0.0-1.0
    int MaxLights = 32;                  // Máximo de luzes
    int ShadowResolution = 2048;         // Resolução de sombras
    
    // Performance
    bool EnableGPUCulling = true;        // Culling na GPU
    bool EnableAsyncCompute = false;     // Compute assíncrono (limitado)
    
    // Debug
    bool ShowDebugInfo = false;          // Debug info
    bool WireframeMode = false;          // Wireframe
};
```

---

## ?? Modos de Renderização

### Padrão (DEFAULT)
```cpp
gAMDRender->Initialize(CAMDRenderSystem::GetDefaultConfig());

// Deferred + 32 luzes + Wave64 + LDS
// Melhor balanço performance/qualidade
```

### Performance
```cpp
CAMDRenderSystem::RenderConfig config;
config.DetailLevel = 0.5f;
config.MaxLights = 16;
config.ShadowResolution = 1024;
config.EnableWaveOptimization = true;
gAMDRender->Initialize(config);

// Renderização mais rápida, menos luzes
```

### Qualidade
```cpp
CAMDRenderSystem::RenderConfig config;
config.DetailLevel = 1.0f;
config.MaxLights = 64;
config.ShadowResolution = 4096;
gAMDRender->Initialize(config);

// Máxima qualidade com mais luzes
```

---

## ?? APIs Disponíveis

### Ciclo de Renderização

```cpp
// Iniciar frame
gAMDRender->BeginFrame();

// Renderização de geometria (Deferred)
gAMDRender->BeginDeferredPass();
// Renderizar meshes...

// Renderização de luzes
gAMDRender->RenderLightPass();

// Finalizar frame
gAMDRender->EndFrame();
```

### Configuração em Tempo Real

```cpp
// Obter configuração atual
const auto& config = gAMDRender->GetRenderConfig();

// Modificar configuração
CAMDRenderSystem::RenderConfig newConfig = config;
newConfig.MaxLights = 64;
gAMDRender->SetRenderConfig(newConfig);
```

### Estatísticas

```cpp
const auto& stats = gAMDRender->GetStats();
std::cout << "Draw Calls: " << stats.DrawCalls << std::endl;
std::cout << "Triangles: " << stats.TrianglesRendered << std::endl;
std::cout << "Lights: " << stats.LightsRendered << std::endl;
std::cout << "Frame Time: " << stats.FrameTimeMS << " ms" << std::endl;

// Resetar stats
gAMDRender->ResetStats();
```

### Debug

```cpp
// Imprimir stats
gAMDRender->PrintStats();

// Renderizar overlay debug
gAMDRender->RenderDebugOverlay();

// Modo wireframe
CAMDRenderSystem::RenderConfig config = gAMDRender->GetRenderConfig();
config.WireframeMode = true;
gAMDRender->SetRenderConfig(config);
```

---

## ?? Performance

### Esperado em AMD RX 6800 (Recomendado)

| Métrica | Valor |
|---------|-------|
| FPS (1440p) | 80-120 |
| Draw Calls | 100-200 |
| GPU Memory | 500-1500 MB |
| Frame Time | 8-12 ms |

### Esperado em AMD RX 5700 (Padrão)

| Métrica | Valor |
|---------|-------|
| FPS (1080p) | 60-100 |
| Draw Calls | 100-150 |
| GPU Memory | 300-800 MB |
| Frame Time | 10-16 ms |

---

## ?? Exemplo Completo

```cpp
// main.cpp

#include "AMD/Core/CAMDRenderSystem.h"

// Inicializar
void InitAMD()
{
    CAMDRenderSystem::RenderConfig config;
    config.MaxLights = 32;
    config.DetailLevel = 0.8f;
    config.EnableWaveOptimization = true;
    
    gAMDRender->Initialize(config);
    gAMDRender->PrintStats();
}

// Loop principal
void MainScene(HDC hDC)
{
    // Iniciar frame
    gAMDRender->BeginFrame();
    
    // Renderizar geometria (G-Buffers)
    gAMDRender->BeginDeferredPass();
    
    // Renderizar seus meshes aqui
    glBindVertexArray(terrainVAO);
    glDrawElements(GL_TRIANGLES, terrainIndexCount, GL_UNSIGNED_INT, nullptr);
    
    // Renderizar luzes
    gAMDRender->RenderLightPass();
    
    // Finalizar frame
    gAMDRender->EndFrame();
    
    // Debug
    gAMDRender->PrintStats();
}

// Finalizar
void ShutdownAMD()
{
    gAMDRender->Shutdown();
}
```

---

## ?? Otimizações Específicas AMD

### Wave Optimization
```cpp
// Habilitado por padrão
config.EnableWaveOptimization = true;

// Benefícios:
// - Wave64 (64 threads por onda)
// - Execução eficiente
// - Melhor utilização de EU (Execution Units)
```

### Local Data Store (LDS)
```cpp
// Habilitado por padrão
config.EnableLDSUsage = true;

// Benefícios:
// - Cache compartilhado 96KB
// - Acesso rápido a dados
// - Reduz latência de memória
```

### Predicate Execution
```cpp
// Habilitado por padrão
config.EnablePredicates = true;

// Benefícios:
// - Execução condicional eficiente
// - Reduz branch misprediction
// - Melhor divergência de threads
```

---

## ?? Troubleshooting

### Problema: Performance Baixa

**Solução:**
```cpp
// Reduzir máximo de luzes
config.MaxLights = 16;  // De 32

// Reduzir resolução de sombra
config.ShadowResolution = 1024;  // De 2048

// Reduzir detail level
config.DetailLevel = 0.5f;  // De 0.8f

gAMDRender->SetRenderConfig(config);
```

### Problema: Artefatos Visuais

**Solução:**
```cpp
// Verificar Wave Optimization
config.EnableWaveOptimization = false;
gAMDRender->SetRenderConfig(config);

// Se isso resolver, é problema de shader
```

### Problema: Alto Consumo de Memória

**Solução:**
```cpp
// Reduzir shadow resolution
config.ShadowResolution = 1024;

// Reduzir máximo de luzes
config.MaxLights = 16;

gAMDRender->SetRenderConfig(config);
```

---

## ?? Estrutura de Pastas

```
source/AMD/
??? Core/
?   ??? CAMDRenderSystem.h
?   ??? CAMDRenderSystem.cpp
?   ??? (CAMDGPUManager - futuro)
?
??? Modules/
?   ??? Terrain/           (A criar)
?   ??? Character/         (A criar)
?   ??? Effects/           (A criar)
?
??? Documentation/
    ??? AMD_RENDER_GUIDE.md (Este arquivo)
    ??? (Mais docs)
```

---

## ?? Próximos Passos

1. **Integrar** sistema AMD no seu projeto
2. **Testar** com GPU AMD
3. **Otimizar** configurações para seu jogo
4. **Criar Modules** (Terrain, Character, etc)
5. **Documentar** otimizações específicas

---

**Status**: ? Sistema pronto
**Versão**: 1.0
**Data**: 2024
**Compatibilidade**: AMD RDNA (RX 6000+), RDNA 2, GCN anterior

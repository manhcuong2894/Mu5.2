# ?? Terrain NVIDIA - Guia Completo

## ?? Introdução

O módulo **Terrain NVIDIA** oferece renderização otimizada de terrenos com:

? LOD (Level of Detail) automático
? Frustum Culling na GPU
? Suporte a Deferred Rendering
? Instancing de geometria
? Compute shaders para processamento
? PBR (Physically Based Rendering)
? Configurações de Performance/Qualidade

---

## ?? Quick Start

### 1. Inicializar

```cpp
#include "NVIDIA/Modules/Terrain/CTerrainNvidia.h"
#include "NVIDIA/Modules/Terrain/TerrainNvidiaConfig.h"

// No WinMain, após inicializar NVIDIA:
gTerrainNvidia->Initialize(TerrainNvidia::DEFAULT_CONFIG);
```

### 2. Adicionar Chunks

```cpp
// Adicionar um chunk de terreno (1024x1024)
GLuint terrainData = LoadTerrainHeightmap("terrain_heightmap.png");
gTerrainNvidia->AddChunk(0, 0, 1024.0f, terrainData);

// Adicionar múltiplos chunks (grid)
for (int x = -2; x <= 2; ++x)
{
    for (int z = -2; z <= 2; ++z)
    {
        gTerrainNvidia->AddChunk(x, z, 1024.0f, LoadHeightmap(x, z));
    }
}
```

### 3. Renderizar

```cpp
// Em seu loop de renderização:
void MainScene(HDC hDC)
{
    glm::mat4 view = CalcViewMatrix();
    glm::mat4 proj = CalcProjMatrix();
    glm::vec3 cameraPos = GetCameraPosition();

    // Atualizar (calcula LOD, culling, etc)
    gTerrainNvidia->Update(cameraPos);

    // Renderizar
    gTerrainNvidia->Render(view, proj);
}
```

---

## ?? Configurações

### Perfil: DEFAULT_CONFIG (Padrão Balanceado)

```cpp
gTerrainNvidia->Initialize(TerrainNvidia::DEFAULT_CONFIG);

// Características:
// - Deferred Rendering: SIM
// - Instancing: SIM
// - Detail Level: 1.0 (máximo)
// - LOD Distance: 300
// - PBR: SIM
```

### Perfil: PERFORMANCE_CONFIG (Otimizado para FPS)

```cpp
gTerrainNvidia->Initialize(TerrainNvidia::PERFORMANCE_CONFIG);

// Características:
// - Deferred Rendering: NÃO
// - Instancing: SIM
// - Detail Level: 0.5 (médio)
// - LOD Distance: 200
// - Shaders mais rápidos
// - Menos texturas
```

### Perfil: QUALITY_CONFIG (Máxima Qualidade)

```cpp
gTerrainNvidia->Initialize(TerrainNvidia::QUALITY_CONFIG);

// Características:
// - Deferred Rendering: SIM
// - Instancing: SIM
// - Detail Level: 1.0 (máximo)
// - LOD Distance: 500
// - Tessellation: SIM
// - Virtual Texturing: SIM
// - 16 texturas simultâneas
```

---

## ?? Configuração Personalizada

```cpp
// Criar configuração customizada
TerrainNvidia::Config myConfig = TerrainNvidia::DEFAULT_CONFIG;

// Ajustar parâmetros
myConfig.EnableDeferred = true;
myConfig.EnableTessellation = true;
myConfig.DetailLevel = 0.8f;
myConfig.LodDistance = 400.0f;
myConfig.MaxTexturesPerMaterial = 12;
myConfig.ShowGPUStats = true;  // Para debug

// Aplicar
gTerrainNvidia->SetConfig(myConfig);
```

---

## ?? Gerenciamento de Chunks

### Adicionar

```cpp
// Um por um
gTerrainNvidia->AddChunk(0, 0, 1024.0f, terrainData);

// Grid automático
for (int x = -5; x <= 5; ++x)
{
    for (int z = -5; z <= 5; ++z)
    {
        GLuint data = LoadTerrainChunk(x, z);
        gTerrainNvidia->AddChunk(x, z, 1024.0f, data);
    }
}
```

### Remover

```cpp
// Remover chunk específico
gTerrainNvidia->RemoveChunk(0, 0);

// Limpar todos
gTerrainNvidia->ClearAllChunks();
```

---

## ?? Materiais

### Definir Material

```cpp
CTerrainNvidia::TerrainMaterial material;

// Texturas
material.AlbedoTexture = LoadTexture("albedo.png");
material.NormalTexture = LoadTexture("normal.png");
material.RoughnessTexture = LoadTexture("roughness.png");
material.MetallicTexture = LoadTexture("metallic.png");
material.AOTexture = LoadTexture("ao.png");

// Propriedades PBR
material.TintColor = glm::vec4(1.0, 1.0, 1.0, 1.0);
material.Roughness = 0.7f;
material.Metallic = 0.1f;

// Aplicar
gTerrainNvidia->SetMaterial(0, material);
```

---

## ?? Performance

### Monitorar Performance

```cpp
// Obter estatísticas
const auto& stats = gTerrainNvidia->GetStats();

std::cout << "Chunks visíveis: " << stats.VisibleChunks 
          << " / " << stats.TotalChunks << std::endl;
std::cout << "Vértices: " << stats.VerticesRendered << std::endl;
std::cout << "Draw Calls: " << stats.DrawCalls << std::endl;
std::cout << "GPU Time: " << stats.GPUTimeMS << " ms" << std::endl;
```

### Ativar Debug Mode

```cpp
// Ver stats detalhadas
gTerrainNvidia->SetDebugMode(true);

// Isso mostrará no console:
// [Terrain NVIDIA] Stats:
//   Chunks visíveis: 9/25
//   Vértices: 1048576
//   Draw calls: 9
```

---

## ?? LOD (Level of Detail)

### Como Funciona

```
Distância < 300m  ? LOD 0 (máxima qualidade)
Distância < 500m  ? LOD 1 (qualidade média)
Distância < 750m  ? LOD 2 (qualidade baixa)
Distância > 750m  ? LOD 3 (mínima qualidade)
```

### Ajustar Distância

```cpp
// Aumentar distância de LOD (menos chunks em alta qualidade)
gTerrainNvidia->SetLODDistance(500.0f);

// Config customizada
auto config = gTerrainNvidia->GetConfig();
config.LodDistance = 400.0f;
gTerrainNvidia->SetConfig(config);
```

---

## ?? Culling

O módulo automaticamente remove chunks fora da câmera:

```cpp
// Frustum Culling é feito automaticamente em Update()
gTerrainNvidia->Update(cameraPos);

// Chunks invisíveis não são renderizados
// Isso reduz draw calls e aumenta FPS
```

---

## ?? Exemplo Completo

```cpp
// ========== Winmain.cpp ==========

#include "NVIDIA/Modules/Terrain/CTerrainNvidia.h"

// Inicialização
void InitTerrain()
{
    // Inicializar
    gTerrainNvidia->Initialize(TerrainNvidia::DEFAULT_CONFIG);
    
    // Ativar debug
    gTerrainNvidia->SetDebugMode(true);

    // Adicionar chunks
    for (int x = -2; x <= 2; ++x)
    {
        for (int z = -2; z <= 2; ++z)
        {
            GLuint heightmap = LoadHeightmap(x, z);
            gTerrainNvidia->AddChunk(x, z, 1024.0f, heightmap);
        }
    }

    // Definir material
    CTerrainNvidia::TerrainMaterial material;
    material.AlbedoTexture = LoadTexture("grass.png");
    material.NormalTexture = LoadTexture("grass_normal.png");
    material.Roughness = 0.8f;
    gTerrainNvidia->SetMaterial(0, material);
}

// Loop de renderização
void MainScene(HDC hDC)
{
    glm::vec3 cameraPos = GetCameraPosition();
    glm::mat4 view = CalcViewMatrix();
    glm::mat4 proj = CalcProjMatrix();

    // Atualizar terrain
    gTerrainNvidia->Update(cameraPos);

    // Renderizar terrain
    gTerrainNvidia->Render(view, proj);

    // Mostrar stats (debug)
    const auto& stats = gTerrainNvidia->GetStats();
    DisplayText(fmt("Chunks: %d/%d | Vertices: %d | Draw Calls: %d",
        stats.VisibleChunks, stats.TotalChunks, 
        stats.VerticesRendered, stats.DrawCalls));
}

// Finalização
void ShutdownTerrain()
{
    gTerrainNvidia->Shutdown();
}
```

---

## ?? Troubleshooting

### Problema: Chunks não aparecem

**Solução:**
```cpp
// Verificar se foram adicionados
std::cout << "Total chunks: " << gTerrainNvidia->GetStats().TotalChunks << std::endl;

// Verificar se estão visíveis
gTerrainNvidia->SetDebugMode(true);
```

### Problema: Performance lenta

**Solução:**
```cpp
// Usar PERFORMANCE_CONFIG
gTerrainNvidia->Initialize(TerrainNvidia::PERFORMANCE_CONFIG);

// Ou ajustar manual
TerrainNvidia::Config config = TerrainNvidia::DEFAULT_CONFIG;
config.DetailLevel = 0.5f;  // Reduzir detalhes
config.LodDistance = 250.0f;  // LOD mais próximo
gTerrainNvidia->SetConfig(config);
```

### Problema: Muitos draw calls

**Solução:**
```cpp
// Ativar instancing
TerrainNvidia::Config config = TerrainNvidia::DEFAULT_CONFIG;
config.EnableInstancing = true;
config.EnableAsyncCompute = true;
gTerrainNvidia->SetConfig(config);
```

---

## ?? Shader Otimizações

O shader `terrain.nvidia.fs` usa:

? **PBR (Physically Based Rendering)**
- Cook-Torrance BRDF
- GGX Distribution
- Schlick Geometry
- Fresnel

? **LOD Optimization**
- Diferentes texturas por LOD
- Redução de samples em distância

? **Normal Mapping**
- Matriz TBN para normal mapping
- Tangent space normals

---

## ?? Arquivos Relacionados

- `TerrainNvidiaConfig.h` - Configurações
- `CTerrainNvidia.h/cpp` - Implementação
- `terrain.nvidia.vs` - Vertex shader
- `terrain.nvidia.fs` - Fragment shader
- `MODULAR_STRUCTURE.md` - Arquitetura geral

---

## ?? Próximas Melhorias

- [ ] Tessellation shader
- [ ] Virtual texturing
- [ ] Ray tracing
- [ ] DLSS integration
- [ ] Real-time editing
- [ ] Foliage rendering

---

**Status**: ? Pronto para produção
**Versão**: 1.0
**Data**: 2024

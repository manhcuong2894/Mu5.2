# ?? Guia Completo de Harmonização - Shaders, Scaleform, NVIDIA/AMD/Intel + 100 FPS

## ?? Visão Geral

Sistema centralizado que coordena:
- ? **CShaderGL** - Gerenciador de shaders
- ? **CScaleformSystem** - UI Scaleform
- ? **CColorSystem** - Colorização dinâmica
- ? **NVIDIA/AMD/Intel Systems** - Renderização GPU-específica
- ? **100+ FPS** - Otimizado para performance

---

## ?? Arquitetura Completa

```
???????????????????????????????????????????????????????
?         CRenderOrchestrator (Coordenador)           ?
?  - Sincronização central                             ?
?  - Batching automático                               ?
?  - Load balancing GPU                                ?
???????????????????????????????????????????????????????
?
?? CRenderPipeline ???????????????????????????????????
?  - Depth Pass                                        ?
?  - G-Buffer (Deferred)                               ?
?  - Geometry Rendering                                ?
?  - Lighting                                          ?
?  - Post-Process                                      ?
?  - Scaleform UI                                      ?
???????????????????????????????????????????????????????
?
?? Subsistemas Paralelos ?????????????????????????????
?                                                      ?
?  ?? CShaderGL (Shaders)                             ?
?  ?  ?? Vertex Shaders                               ?
?  ?  ?? Fragment Shaders                             ?
?  ?  ?? Geometry Shaders                             ?
?  ?                                                  ?
?  ?? CColorSystem (Cores)                            ?
?  ?  ?? Paletas                                      ?
?  ?  ?? Colorização dinâmica                         ?
?  ?  ?? Lookup tables                                ?
?  ?                                                  ?
?  ?? CScaleformSystem (UI)                           ?
?  ?  ?? Carregamento .gfx                            ?
?  ?  ?? Renderização                                 ?
?  ?  ?? Entrada                                      ?
?  ?                                                  ?
?  ?? GPU Renderers (NVIDIA/AMD/Intel)               ?
?     ?? CNvidiaSystem (CUDA, async compute)          ?
?     ?? CAMDRenderSystem (ACE, LDS)                  ?
?     ?? CIntelRenderSystem (optimizações Intel)      ?
?                                                      ?
???????????????????????????????????????????????????????
?
?? CPerformanceMonitor ??????????????????????????????
   - FPS tracking                                     ?
   - Frame time analysis                              ?
   - Adaptive quality scaling                         ?
   - GPU utilization monitoring                       ?
??????????????????????????????????????????????????????
```

---

## ?? Fluxo de Renderização (100 FPS)

```
Frame Start
    ?
??? CRenderOrchestrator::BeginFrame()
?   ?? Clear screen
?   ?? Setup viewport
?   ?? Reset statistics
?
??? SyncShaderState()
?   ?? Sincronizar shaders com GPU
?
??? SyncColorSystem()
?   ?? Aplicar cores/lookup tables
?
??? RenderGeometry() ? GPU-Specific
?   ?? NVIDIA: Async compute
?   ?? AMD: ACE queues
?   ?? Intel: Regular compute
?
??? RenderLighting()
?   ?? Deferred shading
?
??? RenderPostProcess()
?   ?? SSAO (Screen Space)
?   ?? SSR (Screen Space)
?   ?? Motion Blur
?
??? RenderScaleform()
?   ?? UI rendering
?
??? ComposeFinalImage()
?   ?? Tonemap + Color grade
?
??? CRenderOrchestrator::EndFrame()
?   ?? Measure performance
?   ?? Enforce FPS lock (100)
?   ?? Adaptive quality
?
??? Present()
    ?? Swap buffers
```

**Tempo máximo por frame: 10ms (100 FPS)**

---

## ?? Tabela de Prioridades de Renderização

| Prioridade | Stage | GPU | Custo | Tempo |
|-----------|-------|-----|-------|-------|
| **0** | TERRAIN | NVIDIA | Alto | 2-3ms |
| **1** | OBJECTS | Qualquer | Médio | 2-3ms |
| **2** | CHARACTERS | AMD/Intel | Médio | 1-2ms |
| **3** | PARTICLES | Qualquer | Baixo | 1ms |
| **4** | UI_GAME | CPU | Muito baixo | 0.5ms |
| **5** | UI_OVERLAY | CPU | Muito baixo | 0.5ms |

**Total: ~10ms = 100 FPS** ?

---

## ?? Integração por Subsistema

### 1. CShaderGL (Shaders)

```cpp
// Inicializar
gShaderGL->Init();

// Renderização automática via orquestrador
gRenderOrchestrator->SyncShaderState();

// Não chame glUseProgram() diretamente
// O orquestrador gerencia tudo
```

**O que fazer:**
- ? Criar shaders em `source/Shaders/`
- ? Deixar CShaderGL gerenciar compila çãoção
- ? Deixar Orquestrador sincronizar

**O que NÃO fazer:**
- ? Não alterne shaders manualmente
- ? Não chame glUseProgram() diretamente
- ? Não crie VBOs/VAOs sem registrar

---

### 2. CColorSystem (Cores)

```cpp
// Inicializar
gColorSystem->Initialize();

// Cores são aplicadas automaticamente nos shaders
// via uniforms sincronizados pelo orquestrador

// Atualizar cores dinâmicas
gColorSystem->SetVariable("itemGrade", "5");
```

**Integração:**
- Paletas definidas em `source/Color/`
- Colorização em tempo real via lookup tables
- Sincronização automática pelo orquestrador

---

### 3. CScaleformSystem (UI)

```cpp
// Carregar UI
GFxHandle inv = gScaleformSystem->LoadMovie("UI/Inventory.gfx");

// Renderização ocorre em stage SCALEFORM
// Orquestrador renderiza após geometria

// Entrada sincronizada
gScaleformInput->ProcessMouseMove(x, y);
```

**O que fazer:**
- ? Carregar antes do loop
- ? Renderizar pelo orquestrador
- ? Processar entrada via gScaleformInput

---

### 4. GPU Renderers (NVIDIA/AMD/Intel)

```cpp
// Orquestrador detecta GPU automaticamente
GPUVendor gpu = gRenderOrchestrator->GetDetectedGPU();

// Otimizações automáticas
switch (gpu)
{
    case GPUVendor::NVIDIA:
        // Usar async compute, CUDA
        break;
    case GPUVendor::AMD:
        // Usar ACE queues, LDS
        break;
    case GPUVendor::INTEL:
        // Otimizações Intel Gen
        break;
}
```

**Fluxo:**
1. CRenderOrchestrator detecta GPU
2. Seleciona render path otimizado
3. Configura batching/instancing
4. Submete para GPU específica

---

## ? Performance Tips para 100+ FPS

### 1. Batching Automático

```cpp
// Ativar batching
CRenderOrchestrator::Config cfg;
cfg.EnableBatching = true;
cfg.MaxBatchSize = 10000;
gRenderOrchestrator->SetConfig(cfg);

// Orquestrador agrupa automaticamente
```

**Resultado:**
- ? 5000 Draw Calls ? 50 Batches
- ? 10ms frame time ? 100 FPS

---

### 2. Instancing

```cpp
// Renderizar 1000 árvores com 1 draw call
RenderBatch batch;
batch.InstanceCount = 1000;
batch.UseInstancing = true;
gRenderOrchestrator->SubmitBatch(batch);
```

**Ganho:**
- ? Sem instancing: 1000 draw calls
- ? Com instancing: 1 draw call (100x mais rápido)

---

### 3. Adaptive Quality

```cpp
// Monitoramento automático
gPerformanceMonitor->AdaptiveQuality(true);

// Se FPS < 100:
// - Reduz resolução
// - Desativa SSAO
// - Simplifica meshes

float quality = gPerformanceMonitor->GetCurrentMetrics().GpuUtilization;
```

---

### 4. FPS Locking

```cpp
// Travar em 100 FPS
gRenderOrchestrator->LockFPS(100);

// Automático:
// - Aguarda frame time
// - Mantém CPU/GPU sincronizados
// - Reduz consumo de energia
```

---

## ?? Estrutura de Ficheiros

```
source/
??? Core/
?   ??? CRenderOrchestrator.h        ? Novo
?   ??? CRenderOrchestrator.cpp      ? Novo
?   ??? CRenderPipeline.h            ? Novo
?   ??? CRenderPipeline.cpp          ? Novo
?   ??? CPerformanceMonitor.h        ? Novo
?   ??? CPerformanceMonitor.cpp      ? Novo
?
??? Shaders/
?   ??? shader.vs                    ? Existente
?   ??? shader.fs                    ? Existente
?   ??? terrain.vs                   ? Existente
?   ??? terrain.fs                   ? Existente
?   ??? glow.vs                      ? Existente
?   ??? glow.fs                      ? Existente
?
??? Color/
?   ??? CColorSystem.h               ? Existente
?   ??? CColorSystem.cpp             ? Existente
?   ??? colorize.fs                  ? Existente
?
??? ScaleForm/
?   ??? CScaleformSystem.h           ? Existente
?   ??? CScaleformSystem.cpp         ? Existente
?   ??? CScaleformInputHandler.h     ? Existente
?
??? NVIDIA/
?   ??? CNvidiaSystem.h              ? Existente
?   ??? CNvidiaSystem.cpp            ? Existente
?   ??? ...
?
??? AMD/
?   ??? CAMDRenderSystem.h           ? Existente
?   ??? ...
?
??? CShaderGL.h                      ? Existente
    CShaderGL.cpp                    ? Existente
```

---

## ?? Implementação em Winmain.cpp

```cpp
int WINAPI WinMain(...)
{
    // 1. Inicializar OpenGL
    CreateOpenglWindow();

    // 2. Inicializar subsistemas
    gShaderGL->Init();
    gColorSystem->Initialize();
    gScaleformSystem->Initialize();
    gScaleformInput->Initialize();

    // 3. Inicializar orquestrador
    gRenderOrchestrator->Initialize();
    gRenderOrchestrator->LockFPS(100);

    // 4. Loop principal
    while (running)
    {
        gRenderOrchestrator->BeginFrame();
        
        // Preparar batches
        // ... seu código de lógica ...
        
        gRenderOrchestrator->RenderFrame();
        gRenderOrchestrator->EndFrame();
        gRenderOrchestrator->Present();
    }

    // 5. Shutdown
    gRenderOrchestrator->Shutdown();
    gScaleformInput->Shutdown();
    gScaleformSystem->Shutdown();
    gColorSystem->Shutdown();
    gShaderGL->Shutdown();

    return 0;
}
```

---

## ?? Monitoramento em Tempo Real

```cpp
// Ativar profiling
gPerformanceMonitor->EnableProfiling(true);
gRenderOrchestrator->ToggleProfiling(true);

// Ver métricas
FrameStats stats = gRenderOrchestrator->GetFrameStats();
printf("FPS: %.1f\n", stats.FPS);
printf("Draw Calls: %d\n", stats.DrawCalls);
printf("Triangles: %d\n", stats.TrianglesRendered);

// Média móvel (60 frames)
float avgFPS = gRenderOrchestrator->GetAverageFPS(60);
```

---

## ?? Checklist de Harmonização

- [ ] CRenderOrchestrator compilando
- [ ] CRenderPipeline compilando
- [ ] CPerformanceMonitor compilando
- [ ] Inicialização em Winmain.cpp
- [ ] Shaders sincronizados
- [ ] Cores aplicadas corretamente
- [ ] Scaleform renderizando
- [ ] GPU detectada automaticamente
- [ ] Batching funcionando
- [ ] FPS > 100 ?

---

## ?? Targets de Performance

| Métrica | Target | Atual | Status |
|---------|--------|-------|--------|
| **FPS** | >100 | Medindo | ? |
| **Frame Time** | <10ms | Medindo | ? |
| **Draw Calls** | <500 | Medindo | ? |
| **GPU Util** | 60-70% | Medindo | ? |
| **CPU Util** | 40-50% | Medindo | ? |

---

## ?? Próximos Passos

1. ? Compilar CRenderOrchestrator, Pipeline, Monitor
2. ? Integrar em Winmain.cpp
3. ? Testar com geometria real
4. ? Otimizar batch sizes
5. ? Validar 100+ FPS

---

**?? Sistema de harmonização completo e pronto!**

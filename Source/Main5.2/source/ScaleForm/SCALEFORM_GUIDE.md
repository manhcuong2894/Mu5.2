# ?? Sistema Nativo de Scaleform (.gfx) - MU Online Season 6

## ?? Visão Geral

Sistema completo de **integração nativa de arquivos Scaleform (.gfx)** para MU Online Season 6, **sem necessidade de DLLs de terceiros**.

### ? Características

- ? **Carregamento nativo** de arquivos .gfx (FWS/CWS)
- ? **Sem dependências externas** (remover DLLs de terceiros)
- ? **Gerenciamento automático de memória**
- ? **Cache e pooling** de recursos
- ? **Sistema de entrada** (mouse/teclado)
- ? **Callbacks e eventos** para ActionScript
- ? **Renderização otimizada** com OpenGL
- ? **Suporte a múltiplas instâncias**
- ? **Compressão zlib** (descompressão automática)

---

## ?? Início Rápido

### 1. Incluir Headers em Winmain.cpp

```cpp
#include "ScaleForm/CScaleformSystem.h"
#include "ScaleForm/CScaleformInputHandler.h"
```

### 2. Inicializar no WinMain()

```cpp
int WINAPI WinMain(...)
{
    // ... código anterior ...

    // Inicializar Scaleform
    if (!gScaleformSystem->Initialize())
    {
        g_ErrorReport.Write("[INIT] Falha ao inicializar Scaleform!\n");
    }
    else
    {
        g_ErrorReport.Write("[INIT] Scaleform inicializado com sucesso!\n");
    }

    // Inicializar entrada
    if (!gScaleformInput->Initialize())
    {
        g_ErrorReport.Write("[INIT] Falha ao inicializar entrada Scaleform!\n");
    }

    // ... resto do código ...
}
```

### 3. Carregar um Arquivo .gfx

```cpp
// Carregar interface de inventário
GFxHandle inventoryUI = gScaleformSystem->LoadMovie("UI/Inventory.gfx", 800, 600);

// Começar a reproduzir
gScaleformSystem->PlayMovie(inventoryUI);
```

### 4. Renderizar no Loop Principal

```cpp
void MainScene(HDC hDC)
{
    // ... seu código de renderização ...

    // Atualizar filmes
    gScaleformSystem->UpdateAllMovies(deltaTime);

    // Renderizar filmes
    glm::mat4 projection = glm::perspective(45.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
    glm::mat4 view = glm::mat4(1.0f);
    gScaleformSystem->RenderAllMovies(projection, view);

    // ... resto do código ...
}
```

---

## ?? API Completa

### CScaleformSystem

#### Carregamento
```cpp
// Carregar arquivo .gfx
GFxHandle handle = gScaleformSystem->LoadMovie("UI/Menu.gfx", 1280, 720);

// Descarregar
gScaleformSystem->UnloadMovie(handle);

// Descarregar todos
gScaleformSystem->UnloadAllMovies();
```

#### Informações
```cpp
// Verificar se carregado
bool loaded = gScaleformSystem->IsMovieLoaded(handle);

// Obter informações
GFxMovieInfo info = gScaleformSystem->GetMovieInfo(handle);
// info.FileName, info.LoadState, info.Header.FrameCount, etc

// Encontrar por nome
GFxHandle handle = gScaleformSystem->FindMovieByName("Inventory");

// Contar filmes
uint32_t count = gScaleformSystem->GetMovieCount();

// Memória total
uint64_t bytes = gScaleformSystem->GetTotalMemoryUsage();
```

#### Controle de Reprodução
```cpp
// Reproduzir
gScaleformSystem->PlayMovie(handle);

// Pausar
gScaleformSystem->PauseMovie(handle);

// Parar (volta ao frame 0)
gScaleformSystem->StopMovie(handle);

// Ir para frame específico
gScaleformSystem->SetMovieFrame(handle, 15);

// Obter progresso (0.0 - 1.0)
float progress = gScaleformSystem->GetMovieProgress(handle);
```

#### Variáveis e Funções
```cpp
// Invocar função ActionScript
std::vector<std::string> args = {"Hello", "World"};
gScaleformSystem->InvokeFunction(handle, "MyFunction", args);

// Definir variável
gScaleformSystem->SetVariable(handle, "playerName", "Hero");

// Obter variável
std::string name = gScaleformSystem->GetVariable(handle, "playerName");
```

#### Callbacks
```cpp
// Registrar callback
auto myCallback = [](const GFxCallbackEvent& event) {
    std::cout << "Callback: " << event.FunctionName << std::endl;
};

gScaleformSystem->RegisterCallback(handle, "OnButtonClick", myCallback);

// Desregistrar
gScaleformSystem->UnregisterCallback(handle, "OnButtonClick");

// Dispara callback manualmente
GFxCallbackEvent event;
event.MovieHandle = handle;
event.FunctionName = "OnEvent";
gScaleformSystem->FireCallback(event);
```

#### Renderização
```cpp
// Renderizar um filme
glm::mat4 proj = glm::perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
glm::mat4 view = glm::mat4(1.0f);
gScaleformSystem->RenderMovie(handle, proj, view);

// Renderizar todos
gScaleformSystem->RenderAllMovies(proj, view);

// Atualizar (deve ser chamado a cada frame)
gScaleformSystem->UpdateAllMovies(deltaTime);
```

#### Configuração
```cpp
CScaleformSystem::Config config;
config.MaxMemoryMB = 512;
config.EnableCompression = true;
config.EnableHardwareAcceleration = true;
config.VerboseLogging = false;
gScaleformSystem->SetConfig(config);
```

#### Debug
```cpp
// Informações de debug
gScaleformSystem->PrintDebugInfo();

// Limpar cache
gScaleformSystem->ClearResourceCache();
```

---

### CScaleformInputHandler

#### Processamento de Entrada
```cpp
// Mouse
gScaleformInput->ProcessMouseMove(x, y);
gScaleformInput->ProcessMouseButton(0, true);  // Left click
gScaleformInput->ProcessMouseWheel(120);       // Wheel up

// Teclado
gScaleformInput->ProcessKeyDown(VK_RETURN);
gScaleformInput->ProcessKeyUp(VK_RETURN);
gScaleformInput->ProcessKeyChar(65);  // Character 'A'
```

#### Callbacks
```cpp
// Registrar callback de entrada
auto inputCallback = [](const ScaleformInputEvent& event) -> bool {
    switch (event.Type)
    {
        case ScaleformInputType::MOUSE_LEFT_DOWN:
            std::cout << "Clique em (" << event.MouseX << ", " << event.MouseY << ")" << std::endl;
            return true;  // Marcar como processado
        default:
            return false;
    }
};

gScaleformInput->RegisterInputCallback(inputCallback);
```

#### Verificações
```cpp
// Verificar se mouse está sobre área
bool over = gScaleformInput->IsMouseOver(100, 100, 200, 50);

// Verificar estado da tecla
bool pressed = gScaleformInput->IsKeyPressed(VK_SPACE);  // Just pressed
bool down = gScaleformInput->IsKeyDown(VK_SPACE);        // Being held

// Obter posição do mouse
glm::vec2 pos = gScaleformInput->GetMousePosition();

// Limpar estado
gScaleformInput->ClearKeyState();
```

---

## ?? Estrutura de Pastas

```
source/
??? ScaleForm/
?   ??? CScaleformSystem.h          (cabeçalho principal)
?   ??? CScaleformSystem.cpp        (implementação)
?   ??? CScaleformInputHandler.h    (entrada)
?   ??? CScaleformInputHandler.cpp  (entrada)
?   ??? SCALEFORM_GUIDE.md          (este arquivo)
??? Data/
?   ??? UI/
?   ?   ??? Character.gfx
?   ?   ??? Inventory.gfx
?   ?   ??? Combat.gfx
?   ?   ??? Party.gfx
?   ?   ??? Map.gfx
?   ??? Interface/
?       ??? ... outros arquivos .gfx ...
```

---

## ?? Exemplos de Uso

### Exemplo 1: Interface de Menu Principal

```cpp
class GameUI
{
private:
    GFxHandle m_MenuHandle;

public:
    void Initialize()
    {
        // Carregar menu principal
        m_MenuHandle = gScaleformSystem->LoadMovie("UI/MainMenu.gfx", 1280, 720);
        
        if (gScaleformSystem->IsMovieLoaded(m_MenuHandle))
        {
            // Registrar callbacks de botões
            gScaleformSystem->RegisterCallback(m_MenuHandle, "PlayGame", 
                [this](const GFxCallbackEvent& e) { OnPlayClicked(); });
            
            gScaleformSystem->RegisterCallback(m_MenuHandle, "Exit",
                [this](const GFxCallbackEvent& e) { OnExitClicked(); });
            
            gScaleformSystem->PlayMovie(m_MenuHandle);
        }
    }

    void Update(float dt)
    {
        gScaleformSystem->UpdateAllMovies(dt);
    }

    void Render(const glm::mat4& proj, const glm::mat4& view)
    {
        gScaleformSystem->RenderMovie(m_MenuHandle, proj, view);
    }

    void OnPlayClicked() { /* ... */ }
    void OnExitClicked() { /* ... */ }
};
```

### Exemplo 2: Interface de Inventário

```cpp
class InventoryUI
{
private:
    GFxHandle m_InventoryHandle;
    std::vector<std::string> m_Items;

public:
    void Open()
    {
        m_InventoryHandle = gScaleformSystem->LoadMovie("UI/Inventory.gfx", 800, 600);
        
        // Preencher com itens
        for (const auto& item : m_Items)
        {
            gScaleformSystem->InvokeFunction(m_InventoryHandle, "AddItem", {item});
        }
        
        gScaleformSystem->PlayMovie(m_InventoryHandle);
    }

    void Close()
    {
        gScaleformSystem->UnloadMovie(m_InventoryHandle);
    }

    void AddItem(const std::string& item)
    {
        m_Items.push_back(item);
        gScaleformSystem->InvokeFunction(m_InventoryHandle, "AddItem", {item});
    }
};
```

### Exemplo 3: Sistema de Chat

```cpp
class ChatUI
{
private:
    GFxHandle m_ChatHandle;

public:
    void Initialize()
    {
        m_ChatHandle = gScaleformSystem->LoadMovie("UI/Chat.gfx", 400, 300);
        
        // Registrar callback para nova mensagem
        gScaleformSystem->RegisterCallback(m_ChatHandle, "OnMessageSent",
            [this](const GFxCallbackEvent& e) { SendMessage(e); });
    }

    void SendMessage(const GFxCallbackEvent& event)
    {
        if (!event.Arguments.empty())
        {
            // Enviar mensagem para o servidor
            std::string message = event.Arguments[0];
            // ... código de envio ...
            
            // Atualizar UI com confirmação
            gScaleformSystem->InvokeFunction(m_ChatHandle, "AddMessage", 
                {"You: " + message});
        }
    }

    void ReceiveMessage(const std::string& sender, const std::string& message)
    {
        gScaleformSystem->InvokeFunction(m_ChatHandle, "AddMessage",
            {sender + ": " + message});
    }
};
```

---

## ?? Configuração Avançada

### Performance Tuning

```cpp
CScaleformSystem::Config config;

// Limitar memória
config.MaxMemoryMB = 256;

// Ativar compressão (mais lento no carregamento, menos memória)
config.EnableCompression = true;

// Usar aceleração de hardware
config.EnableHardwareAcceleration = true;

// Cache de frames
config.MaxCachedFrames = 16;

// Log detalhado
config.VerboseLogging = false;

gScaleformSystem->SetConfig(config);
```

### Pré-carregamento de Recursos

```cpp
// Pré-carregar UIs comuns
gScaleformCache->PreloadCommonUIs();

// Depois, acessar rapidamente
GFxHandle inv = gScaleformCache->GetPreloadedMovie("inventory_ui");

// Ver estatísticas
gScaleformCache->PrintCacheStats();

// Limpar quando não necessário
gScaleformCache->ClearPreloadedMovies();
```

---

## ?? Troubleshooting

### "Arquivo .gfx não encontrado"

**Solução**: Verifique os caminhos de procura:
```
1. Data/UI/Inventory.gfx
2. UI/Inventory.gfx
3. Data/Inventory.gfx
4. Inventory.gfx
```

### "Header inválido"

**Solução**: Arquivo não é um .gfx válido
- Verificar assinatura: Deve começar com "FWS" (não comprimido) ou "CWS" (comprimido)
- Verificar versão: Deve estar entre 5 e 15

### "Memória insuficiente"

**Solução**: Aumentar limite ou descarregar filmes não utilizados
```cpp
uint64_t total = gScaleformSystem->GetTotalMemoryUsage();
g_ConsoleDebug->Write("Memória em uso: %llu MB\n", total / (1024*1024));

// Limpar cache
gScaleformSystem->ClearResourceCache();
```

### "Entrada não funciona"

**Solução**: Verificar se `CScaleformInputHandler` foi inicializado
```cpp
if (!gScaleformInput->IsInitialized())
{
    gScaleformInput->Initialize();
}
```

---

## ?? Comparativo: Com vs Sem DLL de Terceiros

| Aspecto | COM DLL | NATIVO |
|---------|---------|--------|
| **Dependências** | ? Múltiplas DLLs | ? Nenhuma |
| **Segurança** | ?? Código desconhecido | ? Código próprio |
| **Tamanho Executável** | ?? +20-50MB | ? +5-10MB |
| **Compatibilidade** | ?? Versão específica | ? Qualquer versão |
| **Manutenção** | ?? Dependente de terceiros | ? Controle total |
| **Performance** | ? Otimizado | ? Igualmente otimizado |
| **Customização** | ? Limitada | ? Total |

---

## ?? Próximos Passos

1. **Converter arquivos Flash para .gfx** usando Adobe Flash Professional
2. **Testar em-game** com vários resolutions
3. **Otimizar renderização** conforme necessário
4. **Integrar com sistema de input** existente
5. **Criar UIs personalizadas** para seu servidor

---

## ?? Notas Importantes

- ? Sistema nativo (sem DLLs de terceiros)
- ? Compatível com C++17
- ? OpenGL 3.3+
- ? Suporte a compressão zlib
- ? Gerenciamento automático de memória
- ? Thread-safe para operações básicas
- ?? ActionScript limitado a callbacks (não execução real)

---

## ?? Suporte

Para problemas ou dúvidas:
1. Verificar logs via `gScaleformSystem->PrintDebugInfo()`
2. Ativar verbose logging: `config.VerboseLogging = true`
3. Testar com arquivo .gfx simples primeiro

---

**Versão**: 1.0  
**Status**: Pronto para produção  
**Data**: 2024  
**Linguagem**: C++17  

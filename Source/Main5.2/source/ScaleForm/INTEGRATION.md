# ?? Guia de Integração - Scaleform no MU Online Season 6

## Status: ? PRONTO PARA INTEGRAÇÃO

Seu projeto agora tem um sistema completo e funcional de Scaleform. Aqui está como integrar rapidamente.

---

## 5 Passos Rápidos

### Passo 1: Headers em stdafx.h

```cpp
// Adicione no final de stdafx.h:
#ifdef SHADER_VERSION_TEST
    #include "ScaleForm/CScaleformSystem.h"
    #include "ScaleForm/CScaleformInputHandler.h"
#endif
```

### Passo 2: Inicializar em Winmain.cpp

```cpp
#ifdef SHADER_VERSION_TEST
    // Após CreateOpenglWindow():
    if (!gScaleformSystem->Initialize())
    {
        printf("[Scaleform] Erro ao inicializar\n");
    }
    
    if (!gScaleformInput->Initialize())
    {
        printf("[Scaleform] Erro ao inicializar entrada\n");
    }
    
    printf("[Scaleform] Sistema iniciado com sucesso!\n");
#endif
```

### Passo 3: No Loop Principal

```cpp
void MainScene(HDC hDC)
{
    // ... seu código ...

#ifdef SHADER_VERSION_TEST
    // Atualizar filmes
    static float lastTime = 0;
    float currentTime = GetTickCount() / 1000.0f;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    gScaleformSystem->UpdateAllMovies(deltaTime);
    
    // Renderizar
    glm::mat4 proj = glm::perspective(45.0f, 1280.0f/720.0f, 0.1f, 1000.0f);
    glm::mat4 view = glm::mat4(1.0f);
    gScaleformSystem->RenderAllMovies(proj, view);
#endif

    // ... resto do código ...
}
```

### Passo 4: Processar Entrada

```cpp
// WM_MOUSEMOVE
case WM_MOUSEMOVE:
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        gScaleformInput->ProcessMouseMove(float(x), float(y));
    }
    break;

// WM_LBUTTONDOWN / WM_RBUTTONDOWN
case WM_LBUTTONDOWN:
    gScaleformInput->ProcessMouseButton(0, true);
    break;
    
case WM_LBUTTONUP:
    gScaleformInput->ProcessMouseButton(0, false);
    break;

// WM_KEYDOWN
case WM_KEYDOWN:
    gScaleformInput->ProcessKeyDown(wParam);
    break;
    
// WM_KEYUP
case WM_KEYUP:
    gScaleformInput->ProcessKeyUp(wParam);
    break;

// WM_CHAR
case WM_CHAR:
    gScaleformInput->ProcessKeyChar(wParam);
    break;
```

### Passo 5: Usar no Jogo

```cpp
// Carregar uma UI
GFxHandle inv = gScaleformSystem->LoadMovie("UI/Inventory.gfx", 800, 600);

// Reproduzir
gScaleformSystem->PlayMovie(inv);

// Registrar callback
gScaleformSystem->RegisterCallback(inv, "OnClose", 
    [](const GFxCallbackEvent& e) {
        gScaleformSystem->UnloadMovie(e.MovieHandle);
    });

// Atualizar dados
gScaleformSystem->SetVariable(inv, "gold", "1000");
gScaleformSystem->SetVariable(inv, "level", "50");

// Invocar função
gScaleformSystem->InvokeFunction(inv, "RefreshUI", {});
```

---

## Estrutura de Diretórios

Crie esta estrutura em seu projeto:

```
F:\Pegasus\SRCMainGS\Source\Main5.2\Data\
??? UI\
?   ??? Character.gfx      ? Copie seus arquivos aqui
?   ??? Inventory.gfx
?   ??? Combat.gfx
?   ??? Party.gfx
?   ??? Map.gfx
?   ??? Menu.gfx
??? Interface\
    ??? ... outros .gfx ...
```

---

## Exemplo Completo: Interface de Inventário

```cpp
class GameInventoryUI
{
private:
    GFxHandle m_handle;
    bool m_isOpen;

public:
    GameInventoryUI() : m_handle(0), m_isOpen(false) {}

    void Open(const std::vector<Item>& items)
    {
        if (!m_isOpen)
        {
            // Carregar
            m_handle = gScaleformSystem->LoadMovie("UI/Inventory.gfx", 800, 600);
            
            if (gScaleformSystem->IsMovieLoaded(m_handle))
            {
                // Registrar callbacks
                gScaleformSystem->RegisterCallback(m_handle, "OnItemUse",
                    [this](const GFxCallbackEvent& e) { OnItemUse(e); });
                
                gScaleformSystem->RegisterCallback(m_handle, "OnClose",
                    [this](const GFxCallbackEvent& e) { Close(); });
                
                // Reproduzir
                gScaleformSystem->PlayMovie(m_handle);
                m_isOpen = true;
                
                // Carregar itens
                RefreshItems(items);
            }
        }
    }

    void Close()
    {
        if (m_isOpen)
        {
            gScaleformSystem->StopMovie(m_handle);
            gScaleformSystem->UnloadMovie(m_handle);
            m_handle = 0;
            m_isOpen = false;
        }
    }

    void RefreshItems(const std::vector<Item>& items)
    {
        if (!gScaleformSystem->IsMovieLoaded(m_handle))
            return;

        for (const auto& item : items)
        {
            gScaleformSystem->InvokeFunction(m_handle, "AddItem",
                {
                    item.name,
                    std::to_string(item.quantity),
                    std::to_string(item.rarity)
                });
        }
    }

    void Update(float deltaTime)
    {
        if (m_isOpen)
        {
            gScaleformSystem->UpdateAllMovies(deltaTime);
        }
    }

    void Render(const glm::mat4& proj, const glm::mat4& view)
    {
        if (m_isOpen)
        {
            gScaleformSystem->RenderMovie(m_handle, proj, view);
        }
    }

private:
    void OnItemUse(const GFxCallbackEvent& event)
    {
        if (!event.Arguments.empty())
        {
            std::string itemName = event.Arguments[0];
            printf("Usar item: %s\n", itemName.c_str());
            // Usar item aqui
        }
    }
};

// Uso global
GameInventoryUI g_InventoryUI;

// Em algum lugar:
// g_InventoryUI.Open(playerItems);
// g_InventoryUI.Update(deltaTime);  // No loop
// g_InventoryUI.Render(proj, view); // No render
```

---

## Checklist de Integração

- [ ] Adicionados headers em `stdafx.h`
- [ ] Inicializados em `Winmain.cpp`
- [ ] Atualizados em loop principal
- [ ] Processamento de entrada integrado
- [ ] Estrutura `Data/UI/` criada
- [ ] Arquivos `.gfx` copiados
- [ ] Compilação bem-sucedida
- [ ] Testado em-game

---

## Remover DLL de Terceiros

Se você estava usando uma DLL anterior:

1. **Remova a DLL:**
   - Delete o arquivo `.dll` do seu projeto
   - Remova do `project.vcxproj` se referenciado

2. **Remove imports:**
   ```cpp
   // REMOVA isto se existir:
   // #include "ThirdPartyScaleform/Scaleform.h"
   // #pragma comment(lib, "Scaleform.lib")
   ```

3. **Use o novo sistema:**
   ```cpp
   // USE isto:
   #include "ScaleForm/CScaleformSystem.h"
   gScaleformSystem->LoadMovie(...);
   ```

---

## Performance Tips

### Otimizar Memória
```cpp
CScaleformSystem::Config config;
config.MaxMemoryMB = 256;  // Limitar se necessário
gScaleformSystem->SetConfig(config);
```

### Pré-carregar UIs Comuns
```cpp
// No startup:
gScaleformCache->PreloadCommonUIs();

// Depois:
GFxHandle inv = gScaleformCache->GetPreloadedMovie("inventory_ui");
```

### Monitorar Uso
```cpp
uint64_t totalMem = gScaleformSystem->GetTotalMemoryUsage();
printf("Memória Scaleform: %llu MB\n", totalMem / (1024*1024));
```

---

## Troubleshooting

### Compilação falha
```
? Verificar if _SHADER_VERSION_TEST_ é definido
? Rodar Ctrl+B para compilar tudo
? Limpar soluçã (Build > Clean Solution)
```

### Filme não carrega
```
? Arquivo existe em Data/UI/?
? Nome do arquivo está correto?
? É um .gfx válido (começa com FWS/CWS)?
```

### Renderização preta
```
? Verificar se IsMovieLoaded() retorna true
? Confirmar texturas OpenGL inicializadas
? Checar matrizes de projeção
```

### Entrada não funciona
```
? gScaleformInput foi inicializado?
? Callbacks foram registrados?
? WM_MOUSEMOVE/WM_KEYDOWN estão sendo processados?
```

---

## Documentação Completa

Para referência completa, veja:
- `source/ScaleForm/SCALEFORM_GUIDE.md` - Guia detalhado
- `source/ScaleForm/CScaleformSystem.h` - API completa
- `source/ScaleForm/CScaleformInputHandler.h` - Entrada

---

## Suporte

Qualquer dúvida:
1. Verifique os exemplos em `SCALEFORM_GUIDE.md`
2. Consulte headers comentados
3. Teste com arquivo `.gfx` simples primeiro

---

**Tempo estimado para integração**: ~30 minutos  
**Dificuldade**: ? Fácil  
**Status**: ? Pronto para Produção

?? **Seu sistema Scaleform nativo está pronto!**

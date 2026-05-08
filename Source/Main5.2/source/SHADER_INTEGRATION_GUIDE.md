# Guia de Integração CShaderGL com CColorSystem

## Visão Geral

O `CShaderGL` foi adaptado para suportar:
- **Múltiplos tipos de shader** (default, terrain, glow, character, colorized)
- **Gerenciamento de uniforms** (bool, int, float, vec2, vec3, vec4, mat4)
- **Matriz de perspectiva dinâmica** com `SetPerspective()`
- **Integração com CColorSystem** para colorização dinâmica

## Características Principais

### 1. Tipos de Shader Suportados

```cpp
enum ShaderType {
    SHADER_DEFAULT = 0,      // shader.vs / shader.fs
    SHADER_TERRAIN = 1,      // terrain.vs / terrain.fs
    SHADER_GLOW = 2,         // glow.vs / glow.fs
    SHADER_CHARACTER = 3,    // character.vs / character.fs
    SHADER_COLORIZED = 4     // colorize.vs / colorize.fs
};
```

### 2. Inicialização

```cpp
// Inicializar o sistema de shaders
gShaderGL->Init();

// Configurar perspectiva
gShaderGL->SetPerspective(45.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
```

### 3. Uso em Renderização

```cpp
// Selecionar um shader
gShaderGL->RenderShader(CShaderGL::SHADER_DEFAULT);

// Enviar uniforms
glm::mat4 model = glm::mat4(1.0f);
gShaderGL->setMat4("model", model);

gShaderGL->setVec3("lightDir", 0.5f, -1.0f, 0.5f);
gShaderGL->setFloat("intensity", 1.0f);

// Renderizar seu VAO
glBindVertexArray(myVAO);
glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);
```

### 4. Renderização com Colorização

```cpp
// Usar shader de colorização
gShaderGL->RenderShader(CShaderGL::SHADER_COLORIZED);

// Configurar colorização
gShaderGL->setInt("colorMode", (int)CColorSystem::ColorizeMode::ITEM_GRADE);
gShaderGL->setInt("colorValue", gradeIndex);

// Renderizar
glBindVertexArray(colorizedVAO);
glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);
```

### 5. Exemplo de RenderVertexBuffer Otimizado

```cpp
void BMD::RenderVertexBuffer(int i, Mesh_t* m, int vertex_index, 
                             vec3_t* vertices, vec2_t* textCoords, vec4_t* colors)
{
    if (!m || vertex_index <= 0)
        return;

    // Selecionar shader
    gShaderGL->RenderShader(CShaderGL::SHADER_DEFAULT);
    GLuint shader_id = gShaderGL->GetShaderId();

    if (!shader_id)
        return;

    // Preparar matrizes
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(-CameraPosition[0], -CameraPosition[1], -CameraPosition[2]),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glm::mat4 projection = glm::perspective(
        glm::radians(CameraFOV),
        (float)WindowWidth / (float)WindowHeight,
        CameraViewNear,
        CameraViewFar * 1.4f
    );

    // Enviar matrizes
    gShaderGL->setMat4("model", model);
    gShaderGL->setMat4("view", view);
    gShaderGL->setMat4("projection", projection);

    // Atualizar dados do VBO
    glBindVertexArray(m->VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m->VBO_Vertices);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_index * sizeof(vec3_t), vertices);

    glBindBuffer(GL_ARRAY_BUFFER, m->VBO_TexCoords);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_index * sizeof(vec2_t), textCoords);

    glBindBuffer(GL_ARRAY_BUFFER, m->VBO_Colors);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_index * sizeof(vec4_t), colors);

    // Renderizar
    glDrawElements(GL_TRIANGLES, vertex_index, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}
```

## API Completa

### Inicialização
- `void Init()` - Carrega todos os shaders
- `void Shutdown()` - Libera recursos dos shaders
- `static CShaderGL* Instance()` - Acessa singleton

### Seleção de Shader
- `void RenderShader(ShaderType type = SHADER_DEFAULT)` - Ativa shader
- `bool CheckedShader(ShaderType type = SHADER_DEFAULT) const` - Verifica se existe

### Getters
- `GLuint GetShaderId() const` - Obtém ID do shader default
- `GLuint GetShaderTerrainId() const` - Obtém ID do terrain shader
- `GLuint GetShaderGlowId() const` - Obtém ID do glow shader
- `GLuint GetShaderCharacterId() const` - Obtém ID do character shader
- `GLuint GetShaderColorizedId() const` - Obtém ID do colorize shader

### Carregamento
- `GLuint LoadShaderProgram(const char* vs, const char* fs)` - Carrega novo shader
- `bool readshader(const char* filename, std::string& shader_text)` - Lê arquivo
- `GLuint run_shader(const char* text, GLenum type)` - Compila shader

### Uniforms - Tipos Básicos
- `void setBool(const char* name, bool value) const`
- `void setInt(const char* name, int value) const`
- `void setFloat(const char* name, float value) const`

### Uniforms - Vetores
- `void setVec2(const char* name, float x, float y) const`
- `void setVec3(const char* name, float x, float y, float z) const`
- `void setVec4(const char* name, float x, float y, float z, float w) const`

### Uniforms - Matrizes
- `void setMat4(const char* name, glm::mat4& matrix) const`

### Perspectiva
- `void SetPerspective(float fov, float aspect, float near, float far)` - Define matriz de projeção
- `void run_projection()` - Atualiza uniform de projeção no shader ativo

## Notas de Implementação

1. **Carregamento de Shaders**: Os shaders são carregados de `Shaders/` ou `Data/Shaders/`
2. **Error Handling**: Erros são loggados via `g_ConsoleDebug`
3. **Thread Safety**: Use em contexto OpenGL válido apenas
4. **Compatibilidade**: Requer OpenGL 3.3+ e suporte a VAO/VBO

## Estrutura de Diretórios

```
Shaders/
??? shader.vs / shader.fs       (default)
??? terrain.vs / terrain.fs     (terrain)
??? glow.vs / glow.fs           (glow)
??? character.vs / character.fs (character)
??? colorize.vs / colorize.fs   (colorized)
```

## Exemplo Completo: Renderização com Cores

```cpp
// Inicializar
gShaderGL->Init();

// Em cada frame
{
    // Renderizar com cores dinâmicas
    BMD* model = GetModel();
    Mesh_t* mesh = model->GetMesh(0);
    
    if (mesh && mesh->VAO)
    {
        gShaderGL->RenderShader(CShaderGL::SHADER_COLORIZED);
        
        // Aplicar cor baseada em grade de item
        if (item.grade < CColorSystem::ColorizeMode::ITEM_GRADE)
        {
            gShaderGL->setInt("colorMode", (int)CColorSystem::ColorizeMode::ITEM_GRADE);
            gShaderGL->setInt("colorValue", item.grade);
        }
        
        // Renderizar
        model->RenderVertexBuffer(0, mesh, indexCount, vertices, texcoords, colors);
    }
}

// Cleanup
gShaderGL->Shutdown();
```

## Troubleshooting

### Shader não carrega
- Verifique se o arquivo existe em `Shaders/` ou `Data/Shaders/`
- Verifique logs em `g_ConsoleDebug`

### Uniforms não funcionam
- Certifique-se de que o shader foi selecionado via `RenderShader()`
- Verifique o nome do uniform no código shader

### Renderização preta/vazia
- Verifique se VAO está corretamente bindado
- Confira matrizes de projeção via `SetPerspective()`

## Performance

- **Batch Rendering**: Agrupe múltiplos objetos com mesmo shader
- **Uniform Caching**: Não resete uniforms a cada frame se desnecessário
- **Shader Switching**: Minimize trocas de shader entre draw calls

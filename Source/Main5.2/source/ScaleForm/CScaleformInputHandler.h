#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <functional>

/**
 * @file CScaleformInputHandler.h
 * @brief Sistema de entrada para UI Scaleform (mouse/teclado)
 * @details Processamento de eventos de entrada e interação com filmes
 */

// Tipos de eventos
enum class ScaleformInputType
{
    MOUSE_MOVE = 0,
    MOUSE_LEFT_DOWN = 1,
    MOUSE_LEFT_UP = 2,
    MOUSE_RIGHT_DOWN = 3,
    MOUSE_RIGHT_UP = 4,
    MOUSE_WHEEL_UP = 5,
    MOUSE_WHEEL_DOWN = 6,
    KEY_DOWN = 7,
    KEY_UP = 8,
    KEY_CHAR = 9
};

// Estrutura de evento de entrada
struct ScaleformInputEvent
{
    ScaleformInputType Type;
    int KeyCode;
    float MouseX;
    float MouseY;
    unsigned int MovieHandle;
    bool Handled;
};

/**
 * @class CScaleformInputHandler
 * @brief Gerenciador de entrada para Scaleform
 */
class CScaleformInputHandler
{
public:
    static CScaleformInputHandler* Instance();

    // Inicializar
    bool Initialize();
    void Shutdown();

    // Processar entrada
    void ProcessMouseMove(float x, float y);
    void ProcessMouseButton(int button, bool pressed);
    void ProcessMouseWheel(int delta);
    void ProcessKeyDown(int keyCode);
    void ProcessKeyUp(int keyCode);
    void ProcessKeyChar(unsigned int charCode);

    // Callbacks de entrada
    typedef std::function<bool(const ScaleformInputEvent&)> InputCallback;
    void RegisterInputCallback(InputCallback callback);
    void UnregisterInputCallback();

    // Estado do mouse
    glm::vec2 GetMousePosition() const { return m_MousePosition; }
    bool IsMouseOver(float x, float y, float width, float height) const;

    // Estado do teclado
    bool IsKeyPressed(int keyCode) const;
    bool IsKeyDown(int keyCode) const;
    void ClearKeyState();

    // Configuração
    struct Config
    {
        bool EnableMouseInput = true;
        bool EnableKeyboardInput = true;
        bool EnableWheelInput = true;
        float DoubleClickThreshold = 0.3f;
        bool VerboseLogging = false;
    };

    void SetConfig(const Config& config) { m_Config = config; }
    const Config& GetConfig() const { return m_Config; }

    virtual ~CScaleformInputHandler();

private:
    CScaleformInputHandler();

    static CScaleformInputHandler* m_pInstance;

    bool m_bInitialized;
    Config m_Config;

    glm::vec2 m_MousePosition;
    glm::vec2 m_LastMousePosition;
    std::vector<bool> m_KeyState;
    std::vector<bool> m_KeyPressed;
    float m_LastClickTime;
    int m_LastClickButton;

    InputCallback m_InputCallback;

    void FireInputEvent(const ScaleformInputEvent& event);
};

#define gScaleformInput (CScaleformInputHandler::Instance())

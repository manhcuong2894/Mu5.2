#include "stdafx.h"
#include "CScaleformInputHandler.h"
#include <cstring>
#include <algorithm>

CScaleformInputHandler* CScaleformInputHandler::m_pInstance = nullptr;

CScaleformInputHandler::CScaleformInputHandler()
    : m_bInitialized(false),
      m_MousePosition(0.0f, 0.0f),
      m_LastMousePosition(0.0f, 0.0f),
      m_KeyState(256, false),
      m_KeyPressed(256, false),
      m_LastClickTime(0.0f),
      m_LastClickButton(-1)
{
}

CScaleformInputHandler::~CScaleformInputHandler()
{
    Shutdown();
}

CScaleformInputHandler* CScaleformInputHandler::Instance()
{
    if (!m_pInstance)
        m_pInstance = new CScaleformInputHandler();
    return m_pInstance;
}

bool CScaleformInputHandler::Initialize()
{
    if (m_bInitialized)
        return true;

    std::fill(m_KeyState.begin(), m_KeyState.end(), false);
    std::fill(m_KeyPressed.begin(), m_KeyPressed.end(), false);

    m_bInitialized = true;
    return true;
}

void CScaleformInputHandler::Shutdown()
{
    if (!m_bInitialized)
        return;

    ClearKeyState();
    m_InputCallback = nullptr;
    m_bInitialized = false;
}

void CScaleformInputHandler::ProcessMouseMove(float x, float y)
{
    if (!m_bInitialized || !m_Config.EnableMouseInput)
        return;

    m_LastMousePosition = m_MousePosition;
    m_MousePosition = glm::vec2(x, y);

    ScaleformInputEvent event;
    event.Type = ScaleformInputType::MOUSE_MOVE;
    event.MouseX = x;
    event.MouseY = y;
    event.KeyCode = 0;
    event.Handled = false;

    FireInputEvent(event);
}

void CScaleformInputHandler::ProcessMouseButton(int button, bool pressed)
{
    if (!m_bInitialized || !m_Config.EnableMouseInput)
        return;

    ScaleformInputType eventType;

    switch (button)
    {
        case 0:
            eventType = pressed ? ScaleformInputType::MOUSE_LEFT_DOWN : ScaleformInputType::MOUSE_LEFT_UP;
            break;
        case 1:
            eventType = pressed ? ScaleformInputType::MOUSE_RIGHT_DOWN : ScaleformInputType::MOUSE_RIGHT_UP;
            break;
        default:
            return;
    }

    ScaleformInputEvent event;
    event.Type = eventType;
    event.MouseX = m_MousePosition.x;
    event.MouseY = m_MousePosition.y;
    event.KeyCode = button;
    event.Handled = false;

    FireInputEvent(event);
}

void CScaleformInputHandler::ProcessMouseWheel(int delta)
{
    if (!m_bInitialized || !m_Config.EnableWheelInput)
        return;

    ScaleformInputType eventType = (delta > 0) ? ScaleformInputType::MOUSE_WHEEL_UP : ScaleformInputType::MOUSE_WHEEL_DOWN;

    ScaleformInputEvent event;
    event.Type = eventType;
    event.MouseX = m_MousePosition.x;
    event.MouseY = m_MousePosition.y;
    event.KeyCode = delta;
    event.Handled = false;

    FireInputEvent(event);
}

void CScaleformInputHandler::ProcessKeyDown(int keyCode)
{
    if (!m_bInitialized || !m_Config.EnableKeyboardInput)
        return;

    if (keyCode < 0 || keyCode >= 256)
        return;

    if (!m_KeyState[keyCode])
    {
        m_KeyState[keyCode] = true;
        m_KeyPressed[keyCode] = true;

        ScaleformInputEvent event;
        event.Type = ScaleformInputType::KEY_DOWN;
        event.KeyCode = keyCode;
        event.MouseX = m_MousePosition.x;
        event.MouseY = m_MousePosition.y;
        event.Handled = false;

        FireInputEvent(event);
    }
}

void CScaleformInputHandler::ProcessKeyUp(int keyCode)
{
    if (!m_bInitialized || !m_Config.EnableKeyboardInput)
        return;

    if (keyCode < 0 || keyCode >= 256)
        return;

    if (m_KeyState[keyCode])
    {
        m_KeyState[keyCode] = false;

        ScaleformInputEvent event;
        event.Type = ScaleformInputType::KEY_UP;
        event.KeyCode = keyCode;
        event.MouseX = m_MousePosition.x;
        event.MouseY = m_MousePosition.y;
        event.Handled = false;

        FireInputEvent(event);
    }
}

void CScaleformInputHandler::ProcessKeyChar(unsigned int charCode)
{
    if (!m_bInitialized || !m_Config.EnableKeyboardInput)
        return;

    ScaleformInputEvent event;
    event.Type = ScaleformInputType::KEY_CHAR;
    event.KeyCode = static_cast<int>(charCode);
    event.MouseX = m_MousePosition.x;
    event.MouseY = m_MousePosition.y;
    event.Handled = false;

    FireInputEvent(event);
}

void CScaleformInputHandler::RegisterInputCallback(InputCallback callback)
{
    m_InputCallback = callback;
}

void CScaleformInputHandler::UnregisterInputCallback()
{
    m_InputCallback = nullptr;
}

bool CScaleformInputHandler::IsMouseOver(float x, float y, float width, float height) const
{
    return m_MousePosition.x >= x && m_MousePosition.x <= (x + width) &&
           m_MousePosition.y >= y && m_MousePosition.y <= (y + height);
}

bool CScaleformInputHandler::IsKeyPressed(int keyCode) const
{
    if (keyCode < 0 || keyCode >= 256)
        return false;
    return m_KeyPressed[keyCode];
}

bool CScaleformInputHandler::IsKeyDown(int keyCode) const
{
    if (keyCode < 0 || keyCode >= 256)
        return false;
    return m_KeyState[keyCode];
}

void CScaleformInputHandler::ClearKeyState()
{
    std::fill(m_KeyState.begin(), m_KeyState.end(), false);
    std::fill(m_KeyPressed.begin(), m_KeyPressed.end(), false);
}

void CScaleformInputHandler::FireInputEvent(const ScaleformInputEvent& event)
{
    if (m_InputCallback)
    {
        m_InputCallback(event);
    }

    if (event.Type == ScaleformInputType::KEY_DOWN && event.KeyCode >= 0 && event.KeyCode < 256)
    {
        m_KeyPressed[event.KeyCode] = false;
    }
}

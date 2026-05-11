#pragma once
#include <functional>
#include <cstdint>

/**
 * @enum Modifier
 * @brief Keyboard/mouse modifier bitmask values.
 *
 * These values match GLFW's modifier constants and represent
 * which modifier keys were held during an input event.
 */
enum class Modifier
{
    NoModifier = 0,
    Shift = 1,
    Control = 2,
    Alt = 4,
    Super = 8,
};

/**
 * @enum Action
 * @brief Represents input action states.
 *
 * Matches GLFW action values for button/key events.
 */
enum class Action
{
    Release = 0,
    Press = 1,
    Repeat = 2,
};

/**
 * @enum ButtonCode
 * @brief Mouse button codes mapping to GLFW's button constants.
 */
enum class ButtonCode
{
    Button_0 = 0,
	Button_1 = 1
    // keep adding if needed later
};

/**
 * @enum KeyCode
 * @brief Keyboard key codes matching GLFW's key values.
 */
enum class KeyCode
{
    UNKNOWN = -1,
    Space = 32,
    Key_0 = 48,
    Key_1 = 49,
    Key_E = 69,
    Key_F = 70,
    Key_H = 72,
    Key_I = 73,
    Key_J = 74,
    Key_K = 75,
    Key_L = 76,
    Key_Q = 81,
    Key_R = 82,
    Key_D = 68,
    Key_O = 79,
    Key_Z = 90,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    DEL = 261
};

class IWindow
{
public:
    using KeyCallback = std::function<void(KeyCode, Action, Modifier)>;
    using CursorPosCallback = std::function<void(double, double)>;
    using MouseCallback = std::function<void(ButtonCode, Action, Modifier, double, double)>;
    using ScrollCallback = std::function<void(double, double)>;

    virtual ~IWindow() = default;

    virtual uint32_t getWidth() const = 0;
    virtual uint32_t getHeight() const = 0;

    virtual void setKeyCallback(const KeyCallback& callback) = 0;
    virtual void setCursorPosCallback(const CursorPosCallback& callback) = 0;
    virtual void setMouseCallback(const MouseCallback& callback) = 0;
    virtual void setScrollCallback(const ScrollCallback& callback) = 0;
};

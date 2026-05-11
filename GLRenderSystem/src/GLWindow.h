#pragma once

#include "../../Interfaces/IWindow.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <functional>
#include <string>
#include <string_view>

struct GLFWwindow;

/**
 * @class GLWindow
 * @brief A lightweight wrapper around a GLFW window with GLAD initialization
 *        and user-provided callback support.
 *
 * GLWindow encapsulates:
 *   - GLFW window creation/destruction
 *   - OpenGL context management
 *   - GLAD loading (once per application)
 *   - Object-oriented callback management via `std::function`
 *
 * It provides trampoline functions to convert GLFW's C-style callbacks
 * into C++ member-based callbacks stored per window instance.
 */
class GLWindow : public IWindow
{
public:
    /**
     * @brief Callback signature for keyboard events.
     * @param KeyCode   The key that triggered the event.
     * @param Action    The key action (pressed/released/repeat).
     * @param Modifier  Modifier keys active during the event.
     */
    using KeyCallback = std::function<void(KeyCode, Action, Modifier)>;

    /**
     * @brief Callback signature for cursor movement events.
     * @param double x-position of the cursor.
     * @param double y-position of the cursor.
     */
    using CursorPosCallback = std::function<void(double, double)>;

    /**
     * @brief Callback signature for mouse button events.
     * @param ButtonCode The mouse button pressed or released.
     * @param Action     The button action.
     * @param Modifier   Active modifier keys.
     * @param double     Cursor X position at the moment of the event.
     * @param double     Cursor Y position at the moment of the event.
     */
    using MouseCallback = std::function<void(ButtonCode, Action, Modifier, double, double)>;

    /**
     * @brief Callback signature for scroll wheel events.
     * @param double X scroll offset.
     * @param double Y scroll offset.
     */
    using ScrollCallback = std::function<void(double, double)>;

public:
    /**
     * @brief Creates a GLFW window and initializes GLAD (once globally).
     *
     * @param title  Window title.
     * @param width  Window width in pixels.
     * @param height Window height in pixels.
     */
    GLWindow(std::string_view title, uint32_t width, uint32_t height);

    /**
     * @brief Destroys the GLFW window.
     */
    ~GLWindow();

    /**
     * @brief Gets the window width.
     * @return The width in pixels.
     */
    uint32_t getWidth() const;

    /**
     * @brief Gets the window height.
     * @return The height in pixels.
     */
    uint32_t getHeight() const;

    // Setter functions for callbacks
    void setKeyCallback(const KeyCallback& callback);
    void setCursorPosCallback(const CursorPosCallback& callback);
    void setMouseCallback(const MouseCallback& callback);
    void setScrollCallback(const ScrollCallback& callback);

    /**
     * @brief Returns the internal GLFWwindow pointer.
     * @return Pointer to the GLFW window object.
     */
    GLFWwindow* getGLFWHandle() const;

    /**
     * @brief Checks if the window should close.
     * @return true if the window should close, false otherwise.
     */
    bool shouldClose() const;

    /**
     * @brief Swaps the front and back buffers of the window.
     */
    void swapBuffers();

private:
    // GLFW trampoline friend functions
    friend void glfwKeyTrampoline(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void glfwCursorPosTrampoline(GLFWwindow* window, double x, double y);
    friend void glfwMouseButtonTrampoline(GLFWwindow* window, int button, int action, int mods);
    friend void glfwScrollTrampoline(GLFWwindow* window, double xoffset, double yoffset);

private:
    // Stored user callbacks
    KeyCallback keyCallback;
    CursorPosCallback cursorCallback;
    MouseCallback mouseCallback;
    ScrollCallback scrollCallback;

    GLFWwindow* glfwWindow = nullptr; 
    uint16_t width = 0;              
    uint16_t height = 0;
};

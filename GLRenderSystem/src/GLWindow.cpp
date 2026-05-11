#include "GLWindow.h"

#include <glfw/glfw3.h>
#include <stdexcept>

void glfwKeyTrampoline(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfwCursorPosTrampoline(GLFWwindow* window, double x, double y);
void glfwMouseButtonTrampoline(GLFWwindow* window, int button, int action, int mods);
void glfwScrollTrampoline(GLFWwindow* window, double xoffset, double yoffset);

GLWindow::GLWindow(std::string_view title, uint32_t width, uint32_t height) : width(static_cast<uint16_t>(width)), height(static_cast<uint16_t>(height))
{
    static bool glfwInitialized = false;
    if (!glfwInitialized)
    {
        if (glfwInit() != GLFW_TRUE)
        {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        glfwInitialized = true;
    }

    glfwWindow = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    if (!glfwWindow)
    {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(glfwWindow);

    static bool initGLAD = false;
    if (!initGLAD)
    {
        initGLAD = true;
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    }

    glfwSetWindowUserPointer(glfwWindow, this);
    glfwSetKeyCallback(glfwWindow, glfwKeyTrampoline);
    glfwSetMouseButtonCallback(glfwWindow, glfwMouseButtonTrampoline);
    glfwSetCursorPosCallback(glfwWindow, glfwCursorPosTrampoline);
    glfwSetScrollCallback(glfwWindow, glfwScrollTrampoline);
}

GLWindow::~GLWindow()
{   
    if (glfwWindow)
    {
        glfwDestroyWindow(glfwWindow);
        glfwWindow = nullptr;
    }
}

uint32_t GLWindow::getWidth() const
{
    return width;
}

uint32_t GLWindow::getHeight() const
{
    return height;
}

void GLWindow::setKeyCallback(const KeyCallback& callback)
{
    keyCallback = callback;
}

void GLWindow::setCursorPosCallback(const CursorPosCallback& callback)
{
    cursorCallback = callback;
}

void GLWindow::setMouseCallback(const MouseCallback& callback)
{
    mouseCallback = callback;
}

void GLWindow::setScrollCallback(const ScrollCallback& callback)
{
    scrollCallback = callback;
}

GLFWwindow* GLWindow::getGLFWHandle() const
{
    return glfwWindow;
}

void glfwKeyTrampoline(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    GLWindow* instance = static_cast<GLWindow*>(glfwGetWindowUserPointer(window));
    if (instance && instance->keyCallback)
    {
        KeyCode keyCode = static_cast<KeyCode>(key);
        Action actionEnum = static_cast<Action>(action);
        Modifier modifierEnum = static_cast<Modifier>(mods);

        instance->keyCallback(keyCode, actionEnum, modifierEnum);
    }
}

void glfwCursorPosTrampoline(GLFWwindow* window, double x, double y)
{
    GLWindow* instance = static_cast<GLWindow*>(glfwGetWindowUserPointer(window));
    if (instance && instance->cursorCallback)
    {
        instance->cursorCallback(x, y);
    }
}

void glfwMouseButtonTrampoline(GLFWwindow* window, int button, int action, int mods)
{
    GLWindow* instance = static_cast<GLWindow*>(glfwGetWindowUserPointer(window));
    if (instance && instance->mouseCallback)
    {
        ButtonCode buttonCode = static_cast<ButtonCode>(button);
        Action actionEnum = static_cast<Action>(action);
        Modifier modifierEnum = static_cast<Modifier>(mods);

        double x, y;
        glfwGetCursorPos(window, &x, &y);

        instance->mouseCallback(buttonCode, actionEnum, modifierEnum, x, y);
    }
}

void glfwScrollTrampoline(GLFWwindow* window, double xoffset, double yoffset)
{
    GLWindow* instance = static_cast<GLWindow*>(glfwGetWindowUserPointer(window));
    if (instance && instance->scrollCallback)
    {
        instance->scrollCallback(xoffset, yoffset);
    }
}

bool GLWindow::shouldClose() const
{
    return glfwWindow ? glfwWindowShouldClose(glfwWindow) != 0 : true;
}

void GLWindow::swapBuffers()
{
    if (glfwWindow)
    {
        glfwSwapBuffers(glfwWindow);
    }
}

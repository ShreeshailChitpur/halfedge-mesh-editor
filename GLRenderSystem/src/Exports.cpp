#include "Exports.h"

#include "GLRenderSystem.h"
#include "GLWindow.h"

IRenderSystem* createRenderSystem()
{
    return new GLRenderSystem();
}

IWindow* createWindow(const char* title, uint32_t width, uint32_t height)
{
    return new GLWindow(title, width, height);
}

void waitEvents()
{
    glfwWaitEvents();
}

void swapDisplayBuffers(IWindow* window)
{
    static_cast<GLWindow*>(window)->swapBuffers();
}

bool windowShouldClose(IWindow* window)
{
    return static_cast<GLWindow*>(window)->shouldClose();
}

void destroyRenderSystem(IRenderSystem* renderSystem)
{
    delete renderSystem;
}

void destroyWindow(IWindow* window)
{
    delete window;
}

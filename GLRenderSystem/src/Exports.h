#pragma once

#include <string>
#include <cstdint>

#ifdef OGL_RENDER_SYSTEM_EXPORT
#define OGL_RENDER_SYSTEM_API __declspec(dllexport)
#else
#define OGL_RENDER_SYSTEM_API __declspec(dllimport)
#endif

class IRenderSystem;
class IWindow;

/**
 * @brief Creates a new OpenGL render system instance.
 *
 * Factory function exported with C linkage to ensure ABI stability
 * across module boundaries.
 *
 * Internally creates a GLRenderSystem instance.
 *
 * @return Pointer to a newly allocated IRenderSystem.
 *
 * @note The caller owns the returned object and is responsible
 *       for destroying it.
 */
extern "C" OGL_RENDER_SYSTEM_API IRenderSystem* createRenderSystem();


/**
 * @brief Creates a new window with an OpenGL context.
 *
 * Internally constructs a GLWindow instance and initializes
 * the underlying windowing system (GLFW).
 *
 * @param title Window title.
 * @param width Window width in pixels.
 * @param height Window height in pixels.
 *
 * @return Pointer to a newly allocated IWindow.
 *
 * @note The returned window must be used with the render system
 *       created by createRenderSystem().
 * @note The caller owns the returned object and is responsible
 *       for destroying it.
 *
 * @warning Although the interface uses std::string_view, the
 *          current implementation expects a modifiable string.
 */
extern "C" OGL_RENDER_SYSTEM_API IWindow* createWindow(const char* title, uint32_t width, uint32_t height);

/**
 * @brief Waits for and processes window system events.
 *
 * This function blocks the calling thread until at least one
 * window event is received.
 *
 * Internally maps to glfwWaitEvents().
 */
extern "C" OGL_RENDER_SYSTEM_API void waitEvents();

/**
 * @brief Swaps the front and back buffers of a window.
 *
 * Presents the rendered frame to the screen.
 *
 * @param window Window whose buffers will be swapped.
 *
 * @note The window must be a valid instance created by
 *       createWindow().
 */
extern "C" OGL_RENDER_SYSTEM_API void swapDisplayBuffers(IWindow* window);

/**
 * @brief Checks whether the window should close.
 *
 * Typically becomes true when the user closes the window
 * or an OS-level close event is received.
 *
 * @param window Window to query.
 *
 * @return True if the window should close, false otherwise.
 */
extern "C" OGL_RENDER_SYSTEM_API bool windowShouldClose(IWindow* window);

/**
 * @brief Destroys a render system instance.
 *
 * Properly deallocates memory for a render system created
 * by createRenderSystem().
 *
 * @param renderSystem Pointer to the render system to destroy.
 *
 * @note After calling this function, the pointer is invalid
 *       and should not be used.
 */
extern "C" OGL_RENDER_SYSTEM_API void destroyRenderSystem(IRenderSystem* renderSystem);

/**
 * @brief Destroys a window instance.
 *
 * Properly deallocates memory for a window created
 * by createWindow().
 *
 * @param window Pointer to the window to destroy.
 *
 * @note After calling this function, the pointer is invalid
 *       and should not be used.
 */
extern "C" OGL_RENDER_SYSTEM_API void destroyWindow(IWindow* window);

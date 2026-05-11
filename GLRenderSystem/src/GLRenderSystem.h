#pragma once

#include "../../Interfaces/IRenderSystem.h"

/**
 * @class GLRenderSystem
 * @brief OpenGL fixed-function render system.
 *
 * Concrete implementation of IRenderSystem using legacy OpenGL
 * (immediate mode and fixed-function pipeline).
 *
 * Responsibilities include:
 * - OpenGL state initialization
 * - Viewport and buffer clearing
 * - Rendering geometry using glBegin/glEnd
 * - Managing world, view, and projection matrices
 *
 * @note This implementation relies on deprecated OpenGL functionality
 *       (e.g., glBegin/glEnd, glMatrixMode). It is intended for learning,
 *       prototyping, or legacy support.
 */
class GLRenderSystem : public IRenderSystem 
{
public:
    /**
     * @brief Initializes OpenGL render state.
     *
     * Must be called after a valid GLFW window and OpenGL context
     * have been created.
     *
     * Sets:
     * - Smooth shading model
     * - Depth testing with GL_LEQUAL
     */
    void init(); 

    /**
     * @brief Clears the color and depth buffers.
     *
     * @param r Red clear color component [0, 1]
     * @param g Green clear color component [0, 1]
     * @param b Blue clear color component [0, 1]
     */
    void clearDisplay(float r, float g, float b);

    /**
     * @brief Defines the OpenGL viewport.
     *
     * @param x X coordinate of the lower-left corner (pixels)
     * @param y Y coordinate of the lower-left corner (pixels)
     * @param width Width of the viewport (pixels)
     * @param height Height of the viewport (pixels)
     */

    void setViewport(double x, double y, double width, double height);

    /**
    * @brief Renders vertices as independent triangles.
    *
    * Uses OpenGL immediate mode (GL_TRIANGLES). Each group of three
    * consecutive vertices forms one triangle.
    *
    * The following vertex attributes are used:
    * - Position
    * - Normal
    * - Color
    *
    * @param vertices Vertex list representing a triangle soup.
    */
    void renderTriangleSoup(const std::vector<Vertex>& vertices);

    /**
     * @brief Renders vertices as line segments.
     *
     * Uses OpenGL immediate mode (GL_LINES). Each pair of consecutive
     * vertices defines a line segment.
     *
     * @param vertices Vertex list representing line geometry.
     */

    void renderLines(const std::vector<Vertex>& vertices);

    /**
     * @brief Configures a light source.
     *
     * @param index Light index.
     * @param position Light position in world space.
     * @param Ia Ambient intensity.
     * @param Id Diffuse intensity.
     * @param Is Specular intensity.
     *
     * @warning Currently not implemented.
     */
    void setupLight(uint32_t index, glm::vec3 position, glm::vec3 Ia, glm::vec3 Id,
        glm::vec3 Is);

    /**
     * @brief Enables or disables a light source.
     *
     * @param index Light index.
     * @param enable True to enable, false to disable.
     *
     * @warning Currently not implemented.
     */
    void turnLight(uint32_t index, bool enable);

    /**
     * @brief Sets the world (model) transformation matrix.
     *
     * Marks the model-view matrix as dirty so it will be
     * updated before the next draw call.
     *
     * @param matrix World transformation matrix.
     */
    void setWorldMatrix(const glm::mat4& matrix);
    
     /**
      * @brief Returns the current world transformation matrix.
      *
      * @return Reference to the world matrix.
      */
    const glm::mat4& getWorldMatrix() const;

    /**
     * @brief Sets the view (camera) transformation matrix.
     *
     * Marks the model-view matrix as dirty.
     *
     * @param matrix View transformation matrix.
     */
    void setViewMatrix(const glm::mat4& matrix);

    /**
     * @brief Returns the current view transformation matrix.
     *
     * @return Reference to the view matrix.
     */
    const glm::mat4& getViewMatrix() const;

    /**
     * @brief Sets the projection transformation matrix.
     *
     * Marks the projection matrix as dirty.
     *
     * @param matrix Projection matrix.
     */
    void setProjMatrix(const glm::mat4& matrix);

    /**
     * @brief Returns the current projection matrix.
     *
     * @return Reference to the projection matrix.
     */
    const glm::mat4& getProjMatrix() const;

private:
    /**
     * @brief Applies transformation matrices to OpenGL.
     *
     * Updates:
     * - GL_MODELVIEW matrix using (view * world)
     * - GL_PROJECTION matrix using projection
     *
     * Matrices are only uploaded if marked dirty, minimizing
     * redundant OpenGL state changes.
     */
    void applyMatrices();

private:
    /// World (model) transformation matrix
    glm::mat4 worldMatrix = glm::mat4(1.0f);

    /// View (camera) transformation matrix
    glm::mat4 viewMatrix = glm::mat4(1.0f);

    /// Projection transformation matrix
    glm::mat4 projMatrix = glm::mat4(1.0f);

    /// True if the model-view matrix needs updating
    bool modelViewDirty = true;

    /// True if the projection matrix needs updating
    bool projectionDirty = true;
};

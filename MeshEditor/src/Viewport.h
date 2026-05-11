#pragma once

#include "Camera.h"

/**
 * @struct ray
 * @brief Represents a 3D ray with origin and direction.
 *
 * Used primarily for cursor picking and ray casting from the viewport
 * into world space.
 */
struct ray
{
    /// Ray origin in world space
    glm::vec3 orig{ 0.0f, 0.0f, 0.0f };

    /// Ray direction (normalized)
    glm::vec3 dir{ 0.0f, 0.0f, 1.0f };
};

/**
 * @class Viewport
 * @brief Encapsulates viewport dimensions, projection settings, and camera access.
 *
 * The Viewport class is responsible for:
 * - Computing projection matrices (perspective or orthographic)
 * - Managing viewport size and depth range
 * - Converting screen-space cursor positions into world-space rays
 *
 * It owns a Camera instance that defines the view transformation.
 */
class Viewport
{
public:
    /**
     * @brief Compute the projection matrix for the viewport.
     *
     * Returns a perspective projection matrix when parallel projection
     * is disabled, or an orthographic projection matrix otherwise.
     *
     * @return Projection matrix.
     */
    glm::mat4 calcProjectionMatrix() const;

    /**
     * @brief Set the viewport size in pixels.
     * @param inWidth Viewport width in pixels.
     * @param inHeight Viewport height in pixels.
     */
    void setViewportSize(uint32_t inWidth, uint32_t inHeight);

    /**
     * @brief Set the vertical field of view (in degrees).
     * @param inFOV Field of view in degrees.
     */
    void setFOV(double inFOV);

    /**
     * @brief Set the near clipping plane distance.
     * @param inZNear Near plane distance.
     */
    void setZNear(double inZNear);

    /**
     * @brief Set the far clipping plane distance.
     * @param inZFar Far plane distance.
     */
    void setZFar(double inZFar);

    /**
     * @brief Enable or disable parallel (orthographic) projection.
     * @param use True to enable orthographic projection.
     */
    void setParallelProjection(bool use);

    /**
     * @brief Get the near clipping plane distance.
     * @return Near plane distance.
     */
    double getZNear() const;

    /**
     * @brief Get the far clipping plane distance.
     * @return Far plane distance.
     */
    double getZFar() const;

    /**
     * @brief Get the vertical field of view in degrees.
     * @return Field of view in degrees.
     */
    double getFov() const;

    /**
     * @brief Get the viewport width.
     * @return Width in pixels.
     */
    double getWidth() const;

    /**
     * @brief Get the viewport height.
     * @return Height in pixels.
     */
    double getHeight() const;

    /**
     * @brief Check whether parallel projection is enabled.
     * @return True if orthographic projection is used.
     */
    bool isParallelProjection() const;

    /**
     * @brief Compute a world-space ray from a cursor position.
     *
     * Cursor coordinates are expected in pixel space, with the origin
     * at the top-left corner of the viewport.
     *
     * - Perspective: ray originates at the camera eye
     * - Orthographic: ray originates on the near plane
     *
     * @param x Cursor x-coordinate in pixels.
     * @param y Cursor y-coordinate in pixels.
     * @return World-space ray.
     */
    ray calcCursorRay(double x, double y) const;

    /**
     * @brief Compute the width of the target plane at the near clipping distance.
     * @return Target plane width.
     */
    double calcTargetPlaneWidth() const;

    /**
     * @brief Compute the height of the target plane at the near clipping distance.
     * @return Target plane height.
     */
    double calcTargetPlaneHeight() const;

    /**
     * @brief Compute the viewport aspect ratio.
     * @return Width divided by height.
     */
    double calcAspectRatio() const;

    /**
     * @brief Access the viewport's camera.
     * @return Reference to the camera.
     */
    Camera& getCamera();

    /**
     * @brief Access the viewport's camera (const).
     * @return Const reference to the camera.
     */
    const Camera& getCamera() const;

    /**
     * @brief Adjust the camera to frame a bounding box within the viewport.
     *
     * Repositions the camera so that the entire bounding box defined by
     * min and max is visible on screen. The camera target is set to the
     * center of the bounding box, and the distance is adjusted based on
     * the projection mode (perspective or orthographic).
     *
     * @param min Minimum corner of the bounding box.
     * @param max Maximum corner of the bounding box.
     */
    void zoomToFit(glm::vec3 min, glm::vec3 max);

private:
    /// Near clipping plane distance
    double zNear{ 0.01 };

    /// Far clipping plane distance
    double zFar{ 500.0 };

    /// Vertical field of view (degrees)
    double fovDeg{ 60.0 };

    /// Viewport width (pixels)
    double width{ 1.0 };

    /// Viewport height (pixels)
    double height{ 1.0 };

    /// Projection mode flag (false = perspective, true = orthographic)
    bool useParallelProjection{ false };

    /// Camera associated with this viewport
    Camera camera;
};

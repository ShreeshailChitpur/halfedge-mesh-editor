#pragma once

#include <glm/glm.hpp>

/**
 * @class Camera
 * @brief Represents a 3D camera with orbit, pan, zoom, and view-orientation controls.
 *
 * The Camera class encapsulates a classic "look-at" camera model defined by:
 * - an eye position
 * - a target position
 * - an up direction
 *
 * It provides utilities for computing view matrices, camera direction vectors,
 * transforming the camera in world space, and snapping to common orthographic
 * and isometric views.
 */
class Camera
{
public:
    /**
     * @brief Compute the view matrix for the camera.
     * @return A view matrix equivalent to glm::lookAt(eye, target, up).
     */
    glm::mat4 calcViewMatrix() const;

    /**
     * @brief Compute the normalized forward direction of the camera.
     * @return Unit vector pointing from eye to target.
     */
    glm::vec3 calcForward() const;

    /**
     * @brief Compute the normalized right direction of the camera.
     * @return Unit vector pointing to the camera's right.
     */
    glm::vec3 calcRight() const;

    /**
     * @brief Compute the distance between the eye and the target.
     * @return Euclidean distance from eye to target.
     */
    double distanceFromEyeToTarget() const;

    /**
     * @brief Get the camera eye position.
     * @return Const reference to the eye position.
     */
    const glm::vec3& getEye() const;

    /**
     * @brief Get the camera target position.
     * @return Const reference to the target position.
     */
    const glm::vec3& getTarget() const;

    /**
     * @brief Translate the camera by a world-space offset.
     * @param delta Translation vector applied to both eye and target.
     */
    void translate(glm::vec3 delta);

    /**
     * @brief Set the distance from the eye to the target while preserving direction.
     * @param D New distance from eye to target.
     */
    void setDistanceToTarget(double D);

    /**
     * @brief Apply a homogeneous transformation to the camera.
     *
     * The eye and target are transformed as points (w = 1),
     * while the up vector is transformed as a direction (w = 0).
     *
     * @param trf Transformation matrix.
     */
    void transform(const glm::mat4& trf);

    /**
     * @brief Rotate the camera around a point and axis.
     *
     * Internally implemented as a translate–rotate–translate transform.
     *
     * @param point Pivot point for rotation.
     * @param axis Rotation axis.
     * @param angle Rotation angle in radians.
     */
    void rotate(glm::vec3 point, glm::vec3 axis, double angle);

    /**
     * @brief Explicitly set the eye, target, and up vectors.
     *
     * The up vector is orthonormalized to ensure a valid camera basis.
     *
     * @param newEye New eye position.
     * @param newTarget New target position.
     * @param newUp Approximate up direction.
     */
    void setEyeTargetUp(glm::vec3 newEye, glm::vec3 newTarget, glm::vec3 newUp);

    /**
     * @brief Set the camera to a front view (+Z).
     */
    void setFrontView();

    /**
     * @brief Set the camera to a rear view (-Z).
     */
    void setRearView();

    /**
     * @brief Set the camera to a right view (+X).
     */
    void setRightView();

    /**
     * @brief Set the camera to a left view (-X).
     */
    void setLeftView();

    /**
     * @brief Set the camera to a top view (+Y).
     */
    void setTopView();

    /**
     * @brief Set the camera to a bottom view (-Y).
     */
    void setBottomView();

    /**
     * @brief Set the camera to an isometric view.
     */
    void setIsoView();

    /**
     * @brief Orbit the camera around the target using arcball rotation.
     *
     * Treats the input vectors as points on a virtual unit sphere.
     * Computes the rotation axis and angle between them, then rotates
     * the camera around the target accordingly.
     *
     * @param a Starting point on the unit sphere.
     * @param b Ending point on the unit sphere.
     */
    void orbit(glm::vec3 a, glm::vec3 b);

    /**
    * @brief Pan the camera parallel to the view plane.
    * @param u Horizontal pan amount.
    * @param v Vertical pan amount.
    */
    void pan(double u, double v);

    /**
     * @brief Zoom the camera by adjusting the distance to target.
     * @param factor Zoom factor.
     */
    void zoom(double factor);
private:
    /// Camera eye position in world space
    glm::vec3 eye{ 0,0,1 };

    /// Camera target position in world space
    glm::vec3 target{ 0,0,0 };

    /// Camera up direction (normalized)
    glm::vec3 up{ 0,1,0 };
};

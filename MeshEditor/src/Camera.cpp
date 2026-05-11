#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

glm::mat4 Camera::calcViewMatrix() const
{
	return glm::lookAt(eye, target, up);
}

glm::vec3 Camera::calcForward() const
{
	return glm::normalize(target - eye);
}

glm::vec3 Camera::calcRight() const
{
	return glm::normalize(glm::cross(calcForward(), up));
}

double Camera::distanceFromEyeToTarget() const
{
	return glm::distance(target, eye);
}

const glm::vec3& Camera::getEye() const
{
	return eye;
}

const glm::vec3& Camera::getTarget() const
{
	return target;
}

void Camera::translate(glm::vec3 delta)
{
	eye += delta;
	target += delta;
}

void Camera::setDistanceToTarget(double D)
{
	eye = target - calcForward() * static_cast<float>(D);
}

void Camera::transform(const glm::mat4& trf)
{
	eye = glm::vec3(trf * glm::vec4(eye, 1.0f));
	target = glm::vec3(trf * glm::vec4(target, 1.0f));
	up = glm::normalize(glm::vec3(trf * glm::vec4(up, 0.0f)));
}

void Camera::rotate(glm::vec3 point, glm::vec3 axis, double angle)
{
	translate(-point);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), static_cast<float>(angle), axis);
	transform(rotation);
	translate(point);
}

void Camera::setEyeTargetUp(glm::vec3 newEye, glm::vec3 newTarget, glm::vec3 newUp)
{
	eye = newEye;
	target = newTarget;
	
	glm::vec3 rotationAxis = glm::cross(calcForward(), newUp);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), rotationAxis);
	up = glm::vec3(rotation * glm::vec4(calcForward(), 0.0f));
}

void Camera::setFrontView()
{
	double currentDistance = distanceFromEyeToTarget();
	setEyeTargetUp(target + glm::vec3(0.0f, 0.0f, 1.0f), target, glm::vec3(0.0f, 1.0f, 0.0f));
	setDistanceToTarget(currentDistance);
}

void Camera::setRightView()
{
	glm::vec3 oldTarget = target;
	setFrontView();
	rotate(oldTarget, glm::vec3(0.0f, 1.0f, 0.0f), glm::half_pi<float>());
}

void Camera::setRearView()
{
	glm::vec3 oldTarget = target;
	setFrontView();
	rotate(oldTarget, glm::vec3(0.0f, 1.0f, 0.0f), glm::pi<float>());
}

void Camera::setLeftView()
{
	glm::vec3 oldTarget = target;
	setFrontView();
	rotate(oldTarget, glm::vec3(0.0f, 1.0f, 0.0f), -glm::half_pi<float>());
}

void Camera::setTopView()
{
	glm::vec3 oldTarget = target;
	setFrontView();
	rotate(oldTarget, glm::vec3(1.0f, 0.0f, 0.0f), -glm::half_pi<float>());
}

void Camera::setBottomView()
{
	glm::vec3 oldTarget = target;
	setFrontView();
	rotate(oldTarget, glm::vec3(1.0f, 0.0f, 0.0f), glm::half_pi<float>());
}

void Camera::setIsoView()
{
	glm::vec3 oldTarget = target;
	setFrontView();
	rotate(oldTarget, glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(45.0f));
	rotate(oldTarget, glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(45.0f));
}

void Camera::orbit(glm::vec3 a, glm::vec3 b)
{
	const float angle = glm::angle(a, b);
	
	constexpr float epsilon = 1e-6f;
	if (angle < epsilon)
	{
		return;
	}
	
	const glm::vec3 axis = glm::cross(b, a);
	
	if (glm::length(axis) < epsilon)
	{
		return;
	}
	
	const glm::vec3 normalizedAxis = glm::normalize(axis);
	
	const glm::mat4 viewMatrix = calcViewMatrix();
	const glm::mat3 rotationComponent = glm::mat3(viewMatrix);
	const glm::mat3 toWorldCameraSpace = glm::transpose(rotationComponent);
	const glm::vec3 worldAxis = toWorldCameraSpace * normalizedAxis;
	
	const glm::mat4 orbitMatrix = glm::rotate(glm::mat4(1.0f), angle, worldAxis);
	
	const glm::vec3 eyeOffset = eye - target;
	eye = target + glm::vec3(orbitMatrix * glm::vec4(eyeOffset, 0.0f));
	
	up = glm::normalize(glm::vec3(orbitMatrix * glm::vec4(up, 0.0f)));
}

void Camera::pan(double u, double v)
{
	const glm::vec3 right = calcRight();
	const glm::vec3 upDir = up;

	const glm::vec3 delta =	right * static_cast<float>(u) +	upDir * static_cast<float>(v);

	translate(delta);
}

void Camera::zoom(double factor)
{
	const double newDistance = distanceFromEyeToTarget() * factor;
	setDistanceToTarget(newDistance);
}

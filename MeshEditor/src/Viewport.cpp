#include "Viewport.h"

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Viewport::calcProjectionMatrix() const
{
	if (!useParallelProjection)
	{
		return glm::perspective(glm::radians(static_cast<float>(fovDeg)), static_cast<float>(calcAspectRatio()), static_cast<float>(zNear), static_cast<float>(zFar));
	}
	else
	{
		return glm::ortho(static_cast<float>(-calcTargetPlaneWidth() / 2.0), static_cast<float>(calcTargetPlaneWidth() / 2.0), static_cast<float>(-calcTargetPlaneHeight() / 2.0), static_cast<float>(calcTargetPlaneHeight() / 2.0), static_cast<float>(zNear), static_cast<float>(zFar));
	}
}

void Viewport::setViewportSize(uint32_t inWidth, uint32_t inHeight)
{
	width = static_cast<double>(inWidth);
	height = static_cast<double>(inHeight);
}

void Viewport::setFOV(double inFOV)
{
	fovDeg = inFOV;
}

void Viewport::setZNear(double inZNear)
{
	zNear = inZNear;
}

void Viewport::setZFar(double inZFar)
{
	zFar = inZFar;
}

void Viewport::setParallelProjection(bool use)
{
	useParallelProjection = use;
}

double Viewport::getZNear() const
{
	return zNear;
}

double Viewport::getZFar() const
{
	return zFar;
}

double Viewport::getFov() const
{
	return fovDeg;
}

double Viewport::getWidth() const
{
	return width;
}

double Viewport::getHeight() const
{
	return height;
}

bool Viewport::isParallelProjection() const
{
	return useParallelProjection;
}

double Viewport::calcAspectRatio() const
{
	return width / height;
}

double Viewport::calcTargetPlaneHeight() const
{
	return 2.0 * camera.distanceFromEyeToTarget() * std::tan(glm::radians(fovDeg) / 2.0);
}

double Viewport::calcTargetPlaneWidth() const
{
	return calcTargetPlaneHeight() * calcAspectRatio();
}

Camera& Viewport::getCamera()
{
	return camera;
}

const Camera& Viewport::getCamera() const
{
	return camera;
}

ray Viewport::calcCursorRay(double x, double y) const
{
	glm::mat4 view = camera.calcViewMatrix();
	glm::mat4 projection = calcProjectionMatrix();
	glm::vec4 viewport = glm::vec4(0.0, 0.0, width, height);

	auto unproject = [&view, &projection, &viewport, this](double x, double y, double depth) 
	{
		return glm::unProject(glm::vec3(x, height - y, depth), view, projection, viewport);
	};

	const glm::vec3 a = unproject(x, y, 0.0);
	const glm::vec3 b = unproject(x, y, 1.0);
	return { .orig = a, .dir = glm::normalize(b - a) };
}

void Viewport::zoomToFit(glm::vec3 min, glm::vec3 max)
{
	const glm::vec3 center = (min + max) * 0.5f;
	const float radius = glm::length(max - min) * 0.5f;
	
	const glm::vec3 translation = center - camera.getTarget();
	camera.translate(translation);
	
	if (!isParallelProjection())
	{
		const double halfFov = glm::radians(fovDeg) / 2.0;
		const double distance = radius / std::sin(halfFov);
		
		camera.setDistanceToTarget(distance);
	}
	else
	{
		const double scaleHeight = 2.0 * radius / calcTargetPlaneHeight();
		const double scaleWidth = 2.0 * radius / calcTargetPlaneWidth();
		const double scale = std::max(scaleHeight, scaleWidth);
		
		const double newDistance = camera.distanceFromEyeToTarget() * scale;
		
		camera.setDistanceToTarget(newDistance);
	}
}

#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

class IRenderSystem
{
public:
    virtual ~IRenderSystem() = default;
    virtual void init() = 0;

    virtual void clearDisplay(float r, float g, float b) = 0;
    virtual void setViewport(double x, double y, double width, double height) = 0;

    virtual void renderTriangleSoup(const std::vector<Vertex>& vertices) = 0;
    virtual void renderLines(const std::vector<Vertex>& vertices) = 0;

    virtual void setupLight(
        uint32_t index,
        glm::vec3 position,
        glm::vec3 Ia,
        glm::vec3 Id,
        glm::vec3 Is) = 0;

    virtual void turnLight(uint32_t index, bool enable) = 0;

    virtual void setWorldMatrix(const glm::mat4& matrix) = 0;
    virtual const glm::mat4& getWorldMatrix() const = 0;

    virtual void setViewMatrix(const glm::mat4& matrix) = 0;
    virtual const glm::mat4& getViewMatrix() const = 0;

    virtual void setProjMatrix(const glm::mat4& matrix) = 0;
    virtual const glm::mat4& getProjMatrix() const = 0;
};

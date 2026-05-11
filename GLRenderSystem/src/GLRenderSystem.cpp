#include "GLRenderSystem.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

void GLRenderSystem::init()
{
    glShadeModel(GL_SMOOTH);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

void GLRenderSystem::clearDisplay(float r, float g, float b)
{
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLRenderSystem::setViewport(double x, double y, double width, double height)
{
    glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

void GLRenderSystem::renderTriangleSoup(const std::vector<Vertex>& vertices)
{
    applyMatrices();
    
    glBegin(GL_TRIANGLES);
    for (const auto& vertex : vertices)
    {
        glColor3f(vertex.color.r, vertex.color.g, vertex.color.b);
        glNormal3f(vertex.normal.x, vertex.normal.y, vertex.normal.z);
        glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
    }
    glEnd();
}

void GLRenderSystem::renderLines(const std::vector<Vertex>& vertices)
{
    applyMatrices();
    
    glBegin(GL_LINES);
    for (const auto& vertex : vertices)
    {
        glColor3f(vertex.color.r, vertex.color.g, vertex.color.b);
        glNormal3f(vertex.normal.x, vertex.normal.y, vertex.normal.z);
        glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
    }
    glEnd();
}

void GLRenderSystem::setupLight(uint32_t index, glm::vec3 position, glm::vec3 Ia, glm::vec3 Id, glm::vec3 Is)
{
    // Implement later
}

void GLRenderSystem::turnLight(uint32_t index, bool enable)
{
    // Implement later
}

void GLRenderSystem::setWorldMatrix(const glm::mat4& matrix)
{
    worldMatrix = matrix;
    modelViewDirty = true;
}

const glm::mat4& GLRenderSystem::getWorldMatrix() const
{
    return worldMatrix;
}

void GLRenderSystem::setViewMatrix(const glm::mat4& matrix)
{
    viewMatrix = matrix;
    modelViewDirty = true;
}

const glm::mat4& GLRenderSystem::getViewMatrix() const
{
    return viewMatrix;
}

void GLRenderSystem::setProjMatrix(const glm::mat4& matrix)
{
    projMatrix = matrix;
    projectionDirty = true;
}

const glm::mat4& GLRenderSystem::getProjMatrix() const
{
    return projMatrix;
}

void GLRenderSystem::applyMatrices()
{
    if (modelViewDirty)
    {
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(glm::value_ptr(viewMatrix * worldMatrix));
        modelViewDirty = false;
    }
    
    if (projectionDirty)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(glm::value_ptr(projMatrix));
        projectionDirty = false;
    }
}

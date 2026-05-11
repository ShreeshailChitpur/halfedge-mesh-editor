#include "Viewport.h"
#include "DynamicLibrary.h"
#include "Mesh.h"

#include "../../HalfEdge/src/HalfEdge.h"
#include "../../Interfaces/IWindow.h"
#include "../../Interfaces/IRenderSystem.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

void renderScene(IRenderSystem& rs);

static constexpr float panAmount = 0.1f;
static constexpr float rotationDegrees = 5.0f;
static constexpr float cameraMoveAmount = 0.5f;

void moveCubeWorldSpace(IRenderSystem& rs, glm::vec3 offset);
void moveCubeLocalSpace(IRenderSystem& rs, glm::vec3 offset);
void rotateCube(IRenderSystem& rs, float angleDegrees, glm::vec3 axis);
void resetCube(IRenderSystem& rs);
void frameCube(Viewport& viewport);
glm::vec3 pointOnSphere(double x, double y, double width, double height);
glm::vec2 screenToTargetPlane(double x, double y, double width, double height, double targetPlaneWidth, double targetPlaneHeight);

static IRenderSystem* renderSystem = nullptr;
static Viewport* viewport = nullptr;
static Mesh* mesh = nullptr;
static bool isOrbiting = false;
static glm::vec3 lastArcballPos;
static bool isPanning = false;
static double lastPanU = 0.0;
static double lastPanV = 0.0;
static bool isDeletingFace = false;
static int selectedFaceIndex = -1;
static bool isShowingBoundaryFaces = false;

void onKeyCallback(KeyCode key, Action action, Modifier mods)
{
    if (action != Action::Press || renderSystem == nullptr)
        return;

    if (key == KeyCode::Key_I)
        moveCubeWorldSpace(*renderSystem, glm::vec3(0.0f, panAmount, 0.0f));

    if (key == KeyCode::Key_K)
        moveCubeWorldSpace(*renderSystem, glm::vec3(0.0f, -panAmount, 0.0f));

    if (key == KeyCode::Key_J)
        moveCubeWorldSpace(*renderSystem, glm::vec3(-panAmount, 0.0f, 0.0f));

    if (key == KeyCode::Key_L)
        moveCubeWorldSpace(*renderSystem, glm::vec3(panAmount, 0.0f, 0.0f));

    if (key == KeyCode::Up)
        moveCubeLocalSpace(*renderSystem, glm::vec3(0.0f, panAmount, 0.0f));

    if (key == KeyCode::Down)
        moveCubeLocalSpace(*renderSystem, glm::vec3(0.0f, -panAmount, 0.0f));

    if (key == KeyCode::Left)
        moveCubeLocalSpace(*renderSystem, glm::vec3(-panAmount, 0.0f, 0.0f));

    if (key == KeyCode::Right)
        moveCubeLocalSpace(*renderSystem, glm::vec3(panAmount, 0.0f, 0.0f));

    if (key == KeyCode::Key_Q)
        rotateCube(*renderSystem, rotationDegrees, glm::vec3(0.0f, 0.0f, 1.0f));

    if (key == KeyCode::Key_E)
        rotateCube(*renderSystem, -rotationDegrees, glm::vec3(0.0f, 0.0f, 1.0f));

    if (key == KeyCode::Key_R)
        rotateCube(*renderSystem, rotationDegrees, glm::vec3(1.0f, 0.0f, 0.0f));

    if (key == KeyCode::Key_F)
        rotateCube(*renderSystem, -rotationDegrees, glm::vec3(1.0f, 0.0f, 0.0f));

    if (key == KeyCode::Key_1)
        resetCube(*renderSystem);

    if (key == KeyCode::Key_H && mesh != nullptr)
        mesh->colorHoles();

    if (key == KeyCode::Key_Z && mesh != nullptr && viewport != nullptr)
    {
        const auto& table = mesh->getHalfEdgeTable();
        const auto& vertices = table.getVertices();

        if (!vertices.empty())
        {
            glm::vec3 minBounds = table.getPoint(half_edge::VertexHandle{ 0 });
            glm::vec3 maxBounds = minBounds;

            for (size_t i = 1; i < vertices.size(); ++i)
            {
                glm::vec3 pos = table.getPoint(half_edge::VertexHandle{ static_cast<int64_t>(i) });
                minBounds = glm::min(minBounds, pos);
                maxBounds = glm::max(maxBounds, pos);
            }

            viewport->zoomToFit(minBounds, maxBounds);
            renderSystem->setViewMatrix(viewport->getCamera().calcViewMatrix());
            renderSystem->setProjMatrix(viewport->calcProjectionMatrix());

            std::cout << "Zoom to fit mesh" << std::endl;
        }
    }

    if (key == KeyCode::Key_O && mesh != nullptr)
    {
        isShowingBoundaryFaces = !isShowingBoundaryFaces;
        if (isShowingBoundaryFaces)
        {
            mesh->paintBoundaryFaces();
            std::cout << "Boundary faces: ON" << std::endl;
        }
        else
        {
            mesh->clearBoundaryFaces();
            std::cout << "Boundary faces: OFF" << std::endl;
        }
    }

    if (key == KeyCode::Key_D && mesh != nullptr)
    {
        isDeletingFace = !isDeletingFace;
        if (!isDeletingFace)
            selectedFaceIndex = -1;
        std::cout<<"Face deletion mode: " << (isDeletingFace ? "ON (click to select, DEL to delete)" : "OFF") << std::endl;
    }

    if (key == KeyCode::DEL && mesh != nullptr)
    {
        if (selectedFaceIndex >= 0)
        {
            const auto& faces = mesh->getHalfEdgeTable().getFaces();

            if (selectedFaceIndex < static_cast<int>(faces.size()))
            {
                std::cout << "Deleting face #" << selectedFaceIndex << " (total faces before: " << faces.size() << ")" << std::endl;

                int faceToDelete = selectedFaceIndex;
                selectedFaceIndex = -1;  

                half_edge::FaceHandle fh{ static_cast<int64_t>(faceToDelete) };
                mesh->deleteFace(fh);

                std::cout << "Faces remaining: " << mesh->getHalfEdgeTable().getFaces().size() << std::endl;

                mesh->colorHoles();
            }
            else
            {
                std::cout << "ERROR: Selected face index " << selectedFaceIndex << " is out of bounds (max: " << faces.size() - 1 << ")" << std::endl;
                selectedFaceIndex = -1;
            }
        }
        else
        {
            std::cout << "No face selected. Press D to enter deletion mode, then click a face." << std::endl;
        }
    }

    if (key == KeyCode::F1 && viewport != nullptr)
    {
        viewport->getCamera().setFrontView();
        renderSystem->setViewMatrix(viewport->getCamera().calcViewMatrix());
    }

    if (key == KeyCode::F2 && viewport != nullptr)
    {
        viewport->getCamera().setTopView();
        renderSystem->setViewMatrix(viewport->getCamera().calcViewMatrix());
    }

    if (key == KeyCode::F3 && viewport != nullptr)
    {
        viewport->getCamera().setRearView();
        renderSystem->setViewMatrix(viewport->getCamera().calcViewMatrix());
    }

    if (key == KeyCode::F4 && viewport != nullptr)
    {
        viewport->getCamera().setRightView();
        renderSystem->setViewMatrix(viewport->getCamera().calcViewMatrix());
    }

    if (key == KeyCode::F5 && viewport != nullptr)
    {
        viewport->getCamera().setLeftView();
        renderSystem->setViewMatrix(viewport->getCamera().calcViewMatrix());
    }

    if (key == KeyCode::F6 && viewport != nullptr)
    {
        viewport->getCamera().setBottomView();
        renderSystem->setViewMatrix(viewport->getCamera().calcViewMatrix());
    }

    if (key == KeyCode::F7 && viewport != nullptr)
    {
        viewport->getCamera().setIsoView();
        renderSystem->setViewMatrix(viewport->getCamera().calcViewMatrix());
    }

    if (key == KeyCode::F8 && viewport != nullptr)
    {
        viewport->setParallelProjection(!viewport->isParallelProjection());
        renderSystem->setProjMatrix(viewport->calcProjectionMatrix());
    }

    std::cout << "Key " << static_cast<int>(key) << " pressed" << std::endl;
}

bool rayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t)
{
    const float EPSILON = 1e-8f;

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(rayDir, edge2);
    float a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        return false;  

    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - v0;
    float u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayDir, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = f * glm::dot(edge2, q);

    return t > EPSILON;
}

int pickFace(double screenX, double screenY, const Mesh& mesh, const Viewport& viewport, const glm::mat4& worldMatrix)
{
    double ndcX = (2.0f * screenX) / viewport.getWidth() - 1.0f;
    double ndcY = 1.0f - (2.0f * screenY) / viewport.getHeight();

    glm::vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);

    glm::mat4 invProj = glm::inverse(viewport.calcProjectionMatrix());
    glm::vec4 rayEye = invProj * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::mat4 invView = glm::inverse(viewport.getCamera().calcViewMatrix());
    glm::vec3 rayWorld = glm::vec3(invView * rayEye);
    rayWorld = glm::normalize(rayWorld);

    glm::vec3 rayOrigin = viewport.getCamera().getEye();

    glm::mat4 invWorld = glm::inverse(worldMatrix);
    glm::vec3 rayOriginObj = glm::vec3(invWorld * glm::vec4(rayOrigin, 1.0f));
    glm::vec3 rayDirObj = glm::vec3(invWorld * glm::vec4(rayWorld, 0.0f));
    rayDirObj = glm::normalize(rayDirObj);

    const auto& table = mesh.getHalfEdgeTable();
    const auto& faces = table.getFaces();

    int closestFace = -1;
    float closestT = std::numeric_limits<float>::max();

    for (size_t fi = 0; fi < faces.size(); ++fi)
    {
        const half_edge::FaceHandle fh{ static_cast<int64_t>(fi) };
        const half_edge::HalfEdgeHandle start = table.deref(fh).heh;

        if (start.index == -1)
            continue;

        std::vector<glm::vec3> positions;
        half_edge::HalfEdgeHandle he = start;
        size_t maxIterations = 100;
        size_t iterations = 0;

        do
        {
            const half_edge::VertexHandle vh = table.destVertex(he);
            positions.push_back(table.getPoint(vh));
            he = table.next(he);

            if (he.index == -1 || ++iterations > maxIterations)
                break;
        } while (he.index != start.index);

        if (positions.size() < 3)
            continue;

        for (size_t i = 1; i + 1 < positions.size(); ++i)
        {
            float t;
            if (rayIntersectsTriangle(rayOriginObj, rayDirObj,
                positions[0], positions[i], positions[i + 1], t))
            {
                if (t < closestT)
                {
                    closestT = t;
                    closestFace = static_cast<int>(fi);
                }
            }
        }
    }
    return closestFace;
}

void highlightFace(IRenderSystem& rs, const Mesh& mesh, int faceIndex)
{
    if (faceIndex < 0)
        return;

    const auto& table = mesh.getHalfEdgeTable();
    const auto& faces = table.getFaces();

    if (faceIndex >= static_cast<int>(faces.size()))
        return;

    const half_edge::FaceHandle fh{ static_cast<int64_t>(faceIndex) };
    const half_edge::HalfEdgeHandle start = table.deref(fh).heh;

    if (start.index == -1)
        return;

    std::vector<glm::vec3> positions;
    half_edge::HalfEdgeHandle he = start;
    size_t maxIterations = 100;
    size_t iterations = 0;

    do
    {
        const half_edge::VertexHandle vh = table.destVertex(he);
        positions.push_back(table.getPoint(vh));
        he = table.next(he);

        if (he.index == -1 || ++iterations > maxIterations)
            break;
    } while (he.index != start.index);

    if (positions.size() < 3)
        return;

    const glm::vec3 n = glm::normalize(glm::cross(positions[1] - positions[0], positions[2] - positions[0]));

    const glm::vec3 highlightColor(1.0f, 0.5f, 0.0f);

    std::vector<Vertex> triangles;
    for (size_t i = 1; i + 1 < positions.size(); ++i)
    {
        triangles.push_back({ positions[0], n, highlightColor });
        triangles.push_back({ positions[i], n, highlightColor });
        triangles.push_back({ positions[i + 1], n, highlightColor });
    }

    rs.renderTriangleSoup(triangles);
}

void onCursorPosCallback(double x, double y)
{
    if (isOrbiting && viewport != nullptr)
    {
        const glm::vec3 currentPos = pointOnSphere(x, y, viewport->getWidth(), viewport->getHeight());
        viewport->getCamera().orbit(lastArcballPos, currentPos);
        lastArcballPos = currentPos;

        renderSystem->setViewMatrix(viewport->getCamera().calcViewMatrix());
    }

    if (isPanning && viewport != nullptr)
    {
        const glm::vec2 targetPlanePos = screenToTargetPlane(
            x, y,
            viewport->getWidth(), viewport->getHeight(),
            viewport->calcTargetPlaneWidth(), viewport->calcTargetPlaneHeight()
        );

        const double dU = lastPanU - targetPlanePos.x;
        const double dV = lastPanV - targetPlanePos.y;

        viewport->getCamera().pan(dU, dV);

        lastPanU = targetPlanePos.x;
        lastPanV = targetPlanePos.y;

        renderSystem->setViewMatrix(viewport->getCamera().calcViewMatrix());
    }

    std::cout << "Mouse position: (" << x << ", " << y << ")" << std::endl;
}

void onMouseButtonCallback(ButtonCode button, Action action, Modifier mods, double x, double y)
{
    if (button == ButtonCode::Button_0 && action == Action::Press && isDeletingFace && mesh != nullptr && viewport != nullptr)
    {
        const auto& faces = mesh->getHalfEdgeTable().getFaces();
        selectedFaceIndex = pickFace(x, y, *mesh, *viewport, renderSystem->getWorldMatrix());

        if (selectedFaceIndex >= 0 && selectedFaceIndex < static_cast<int>(faces.size()))
        {
            std::cout << "Selected face #" << selectedFaceIndex << " (press DELETE to remove)" << std::endl;
        }
        else if (selectedFaceIndex >= 0)
        {
            std::cout << "ERROR: Picked face index " << selectedFaceIndex << " is out of bounds (max: " << faces.size() - 1 << ")" << std::endl;
            selectedFaceIndex = -1;
        }
        else
        {
            std::cout << "No face under cursor" << std::endl;
            selectedFaceIndex = -1;
        }
        return;  
    }

    if (button == ButtonCode::Button_0 && viewport != nullptr)
    {
        if (action == Action::Press)
        {
            isOrbiting = true;
            lastArcballPos = pointOnSphere(x, y, viewport->getWidth(), viewport->getHeight());
        }
        else if (action == Action::Release)
        {
            isOrbiting = false;
        }
    }

    if (button == ButtonCode::Button_1 && viewport != nullptr)
    {
        if (action == Action::Press)
        {
            isPanning = true;
            const glm::vec2 targetPlanePos = screenToTargetPlane(
                x, y,
                viewport->getWidth(), viewport->getHeight(),
                viewport->calcTargetPlaneWidth(), viewport->calcTargetPlaneHeight()
            );
            lastPanU = targetPlanePos.x;
            lastPanV = targetPlanePos.y;
        }
        else if (action == Action::Release)
        {
            isPanning = false;
        }
    }

    const char* actionStr = (action == Action::Press) ? "pressed" : "released";
    std::cout << "Mouse button " << static_cast<int>(button) << " " << actionStr << " at (" << x << ", " << y << ")" << std::endl;
}

void onScrollCallback(double xoffset, double yoffset)
{
    if (viewport != nullptr)
    {
        const double zoomFactor = (yoffset > 0) ? 0.9 : 1.1;
        viewport->getCamera().zoom(zoomFactor);
        renderSystem->setViewMatrix(viewport->getCamera().calcViewMatrix());
    }

    std::cout << "Scroll: (" << xoffset << ", " << yoffset << ")" << std::endl;
}

void addQuad(std::vector<Vertex>& vertices, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 normal, glm::vec3 color)
{
    vertices.push_back({ v0, normal, color });
    vertices.push_back({ v1, normal, color });
    vertices.push_back({ v2, normal, color });

    vertices.push_back({ v0, normal, color });
    vertices.push_back({ v2, normal, color });
    vertices.push_back({ v3, normal, color });
}

std::vector<Vertex> createCubeGeometry(float halfSize, glm::vec3 color)
{
    std::vector<Vertex> vertices;
    vertices.reserve(36);

    const float s = halfSize;

    addQuad(vertices, { -s, -s,  s }, { s, -s,  s }, { s,  s,  s }, { -s,  s,  s }, { 0.0f, 0.0f, 1.0f }, color);
    addQuad(vertices, { s, -s, -s }, { -s, -s, -s }, { -s,  s, -s }, { s,  s, -s }, { 0.0f, 0.0f, -1.0f }, color);
    addQuad(vertices, { -s,  s,  s }, { s,  s,  s }, { s,  s, -s }, { -s,  s, -s }, { 0.0f, 1.0f, 0.0f }, color);
    addQuad(vertices, { -s, -s, -s }, { s, -s, -s }, { s, -s,  s }, { -s, -s,  s }, { 0.0f, -1.0f, 0.0f }, color);
    addQuad(vertices, { s, -s,  s }, { s, -s, -s }, { s,  s, -s }, { s,  s,  s }, { 1.0f, 0.0f, 0.0f }, color);
    addQuad(vertices, { -s, -s, -s }, { -s, -s,  s }, { -s,  s,  s }, { -s,  s, -s }, { -1.0f, 0.0f, 0.0f }, color);

    return vertices;
}

void renderScene(IRenderSystem& rs)
{
    rs.clearDisplay(0.2f, 0.2f, 0.2f);

    constexpr float cubeHalfSize = 1.0f;
    constexpr glm::vec3 cubeColor(0.0f, 1.0f, 0.0f);
    static const std::vector<Vertex> cubeVertices = createCubeGeometry(cubeHalfSize, cubeColor);

    rs.renderTriangleSoup(cubeVertices);
}

void moveCubeWorldSpace(IRenderSystem& rs, glm::vec3 offset)
{
    glm::mat4 worldMatrix = rs.getWorldMatrix();
    worldMatrix = glm::translate(glm::mat4(1.0f), offset) * worldMatrix;
    rs.setWorldMatrix(worldMatrix);
}

void moveCubeLocalSpace(IRenderSystem& rs, glm::vec3 offset)
{
    glm::mat4 worldMatrix = rs.getWorldMatrix();
    worldMatrix = worldMatrix * glm::translate(glm::mat4(1.0f), offset);
    rs.setWorldMatrix(worldMatrix);
}

void rotateCube(IRenderSystem& rs, float angleDegrees, glm::vec3 axis)
{
    glm::mat4 worldMatrix = rs.getWorldMatrix();
    worldMatrix = glm::rotate(worldMatrix, glm::radians(angleDegrees), axis);
    rs.setWorldMatrix(worldMatrix);
}

void resetCube(IRenderSystem& rs)
{
    rs.setWorldMatrix(glm::mat4(1.0f));
}

void frameCube(Viewport& viewport)
{
    constexpr float cubeHalfSize = 1.0f;

    const glm::mat4 worldMatrix = renderSystem->getWorldMatrix();

    std::vector<glm::vec3> corners = 
    {
        glm::vec3(worldMatrix * glm::vec4(-cubeHalfSize, -cubeHalfSize, -cubeHalfSize, 1.0f)),
        glm::vec3(worldMatrix * glm::vec4(cubeHalfSize, -cubeHalfSize, -cubeHalfSize, 1.0f)),
        glm::vec3(worldMatrix * glm::vec4(-cubeHalfSize,  cubeHalfSize, -cubeHalfSize, 1.0f)),
        glm::vec3(worldMatrix * glm::vec4(cubeHalfSize,  cubeHalfSize, -cubeHalfSize, 1.0f)),
        glm::vec3(worldMatrix * glm::vec4(-cubeHalfSize, -cubeHalfSize,  cubeHalfSize, 1.0f)),
        glm::vec3(worldMatrix * glm::vec4(cubeHalfSize, -cubeHalfSize,  cubeHalfSize, 1.0f)),
        glm::vec3(worldMatrix * glm::vec4(-cubeHalfSize,  cubeHalfSize,  cubeHalfSize, 1.0f)),
        glm::vec3(worldMatrix * glm::vec4(cubeHalfSize,  cubeHalfSize,  cubeHalfSize, 1.0f))
    };

    glm::vec3 minBounds = corners[0];
    glm::vec3 maxBounds = corners[0];

    for (const auto& corner : corners)
    {
        minBounds = glm::min(minBounds, corner);
        maxBounds = glm::max(maxBounds, corner);
    }

    viewport.zoomToFit(minBounds, maxBounds);

    renderSystem->setViewMatrix(viewport.getCamera().calcViewMatrix());
    renderSystem->setProjMatrix(viewport.calcProjectionMatrix());
}

glm::vec3 pointOnSphere(double x, double y, double width, double height)
{
    constexpr float epsilon = 1e-6f;
    const float ar = static_cast<float>(width) / static_cast<float>(height);

    const float xInTargetPlane = ((2.0f * static_cast<float>(x) / static_cast<float>(width)) - 1.0f) * ar;
    const float yInTargetPlane = 1.0f - (2.0f * static_cast<float>(y) / static_cast<float>(height));

    glm::vec3 pointOnSphere{ xInTargetPlane, yInTargetPlane, 0.0f };
    const float length = glm::length(pointOnSphere);

    if (length >= 1.0f - epsilon)
    {
        pointOnSphere = glm::normalize(pointOnSphere);
    }
    else
    {
        pointOnSphere.z = std::sqrt(1.0f - std::pow(pointOnSphere.x, 2.0f) - std::pow(pointOnSphere.y, 2.0f));
    }

    return pointOnSphere;
}

glm::vec2 screenToTargetPlane(double x, double y, double width, double height, double targetPlaneWidth, double targetPlaneHeight)
{
    const double u = ((x / width) - 0.5) * targetPlaneWidth;
    const double v = (0.5 - (y / height)) * targetPlaneHeight;

    return glm::vec2(u, v);
}

half_edge::HalfEdgeTable createCube()
{
    half_edge::HalfEdgeTable table;

    const auto vh0 = table.addVertex(glm::vec3(-1.0f, -1.0f, -1.0f));
    const auto vh1 = table.addVertex(glm::vec3(1.0f, -1.0f, -1.0f));
    const auto vh2 = table.addVertex(glm::vec3(-1.0f, 1.0f, -1.0f));
    const auto vh3 = table.addVertex(glm::vec3(1.0f, 1.0f, -1.0f));
    const auto vh4 = table.addVertex(glm::vec3(-1.0f, -1.0f, 1.0f));
    const auto vh5 = table.addVertex(glm::vec3(1.0f, -1.0f, 1.0f));
    const auto vh6 = table.addVertex(glm::vec3(-1.0f, 1.0f, 1.0f));
    const auto vh7 = table.addVertex(glm::vec3(1.0f, 1.0f, 1.0f));

    table.addFace(vh0, vh1, vh3, vh2);
    table.addFace(vh5, vh4, vh6, vh7);
    table.addFace(vh4, vh0, vh2, vh6);
    table.addFace(vh1, vh5, vh7, vh3);
    table.addFace(vh2, vh3, vh7, vh6);
    table.addFace(vh4, vh5, vh1, vh0);
    table.connectTwins();

    return table;
}

half_edge::HalfEdgeTable loadSTL(std::string_view filename)
{
    half_edge::HalfEdgeTable table;

    std::ifstream file(filename.data(), std::ios::binary);
    if (!file)
    {
        throw std::runtime_error(std::string("Failed to open file: ") + filename.data());
    }

    char header[6] = {};
    file.read(header, 5);
    file.seekg(0);

    if (std::string(header) != "solid")
    {
        throw std::runtime_error("Binary STL format not supported");
    }

    constexpr float epsilon = 1e-6f;

    struct QuantizedVec3
    {
        int64_t x, y, z;

        bool operator==(const QuantizedVec3& other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct QuantizedHasher
    {
        std::size_t operator()(const QuantizedVec3& v) const
        {
            std::size_t h1 = std::hash<int64_t>{}(v.x);
            std::size_t h2 = std::hash<int64_t>{}(v.y);
            std::size_t h3 = std::hash<int64_t>{}(v.z);

            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    std::unordered_map<QuantizedVec3, half_edge::VertexHandle, QuantizedHasher> vertexMap;

    auto quantize = [epsilon](const glm::vec3& v) -> QuantizedVec3
        {
            return {
                static_cast<int64_t>(std::llround(v.x / epsilon)),
                static_cast<int64_t>(std::llround(v.y / epsilon)),
                static_cast<int64_t>(std::llround(v.z / epsilon))
            };
        };

    auto getOrCreateVertex = [&vertexMap, &table, &quantize](const glm::vec3& v) -> half_edge::VertexHandle
        {
            QuantizedVec3 key = quantize(v);

            auto it = vertexMap.find(key);
            if (it != vertexMap.end())
                return it->second;

            auto vh = table.addVertex(v);
            vertexMap[key] = vh;
            return vh;
        };

    auto parseVertex = [](const std::string& line) -> glm::vec3
        {
            std::istringstream iss(line);
            std::string keyword;
            float x, y, z;
            iss >> keyword >> x >> y >> z;
            return glm::vec3(x, y, z);
        };

    auto trim = [](std::string& str)
        {
            const size_t start = str.find_first_not_of(" \t\r\n");
            if (start == std::string::npos)
            {
                str.clear();
                return;
            }
            const size_t end = str.find_last_not_of(" \t\r\n");
            str = str.substr(start, end - start + 1);
        };

    std::string line;
    while (std::getline(file, line))
    {
        trim(line);

        if (line.find("facet normal") != std::string::npos)
        {
            std::vector<glm::vec3> vertices;
            vertices.reserve(3);

            while (std::getline(file, line))
            {
                trim(line);

                if (line.find("vertex") != std::string::npos)
                {
                    vertices.push_back(parseVertex(line));
                }
                else if (line.find("endloop") != std::string::npos)
                {
                    break;
                }
            }

            if (vertices.size() == 3)
            {
                auto v0 = getOrCreateVertex(vertices[0]);
                auto v1 = getOrCreateVertex(vertices[1]);
                auto v2 = getOrCreateVertex(vertices[2]);

                table.addFace(v0, v1, v2);
            }
        }
    }

    table.connectTwins();
    return table;
}

int main()
{
    DynamicLibrary dll("GLRenderSystem.dll");

    auto createRenderSystem = dll.getSymbol<IRenderSystem * (*)()>("createRenderSystem");
    auto createWindow = dll.getSymbol<IWindow * (*)(const char* title, uint32_t width, uint32_t height)>("createWindow");
    auto destroyRenderSystem = dll.getSymbol<void(*)(IRenderSystem*)>("destroyRenderSystem");
    auto destroyWindow = dll.getSymbol<void(*)(IWindow*)>("destroyWindow");
    auto waitEvents = dll.getSymbol<void(*)()>("waitEvents");
    auto swapDisplayBuffers = dll.getSymbol<void(*)(IWindow * window)>("swapDisplayBuffers");
    auto windowShouldClose = dll.getSymbol<bool(*)(IWindow * window)>("windowShouldClose");

    auto rs = std::unique_ptr<IRenderSystem, decltype(destroyRenderSystem)>(createRenderSystem(), destroyRenderSystem);
    auto window = std::unique_ptr<IWindow, decltype(destroyWindow)>(createWindow("Not A Window", 640, 480), destroyWindow);

    renderSystem = rs.get();

    Viewport viewportInstance;
    viewportInstance.setViewportSize(window->getWidth(), window->getHeight());
    viewportInstance.setFOV(60.0);
    viewportInstance.setZNear(0.1);
    viewportInstance.setZFar(500.0);
    viewportInstance.getCamera().setEyeTargetUp(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    viewport = &viewportInstance;

    window->setKeyCallback(onKeyCallback);
    window->setCursorPosCallback(onCursorPosCallback);
    window->setMouseCallback(onMouseButtonCallback);
    window->setScrollCallback(onScrollCallback);

    rs->init();

    rs->setupLight(0, glm::vec3{ 0, 5, 0 }, glm::vec3{ 1, 0, 0 }, glm::vec3{ 0, 1, 0 }, glm::vec3{ 0, 0, 1 });
    rs->turnLight(0, true);

    rs->setWorldMatrix(glm::mat4(1.0f));
    rs->setViewMatrix(viewportInstance.getCamera().calcViewMatrix());
    rs->setProjMatrix(viewportInstance.calcProjectionMatrix());

	//Mesh meshInstance(loadSTL("ADD INPUT STL HERE"));
    mesh = &meshInstance;

    while (!windowShouldClose(window.get()))
    {
        rs->setViewport(0, 0, window->getWidth(), window->getHeight());
        rs->clearDisplay(0.53f, 0.81f, 0.92f);

		mesh->lockForRendering();

        meshInstance.render(*rs);
        meshInstance.renderEdges(*rs);
        meshInstance.renderColoredHoles(*rs);
        meshInstance.renderBoundaryFaces(*rs);

        if (isDeletingFace && selectedFaceIndex >= 0)
        {
            const auto& faces = meshInstance.getHalfEdgeTable().getFaces();
            if (selectedFaceIndex >= static_cast<int>(faces.size()))
            {
                std::cout << "[EARLY CHECK] Invalid face index " << selectedFaceIndex
                    << ", max is " << (faces.size() - 1) << ", clearing!" << std::endl;
                selectedFaceIndex = -1;
            }

            if (selectedFaceIndex < static_cast<int>(faces.size()))
            {
                highlightFace(*rs, meshInstance, selectedFaceIndex);
            }
            else
            {
                selectedFaceIndex = -1;
            }
        }

		mesh->unlockForRendering();

        swapDisplayBuffers(window.get());
        waitEvents();
    }
    return 0;
}

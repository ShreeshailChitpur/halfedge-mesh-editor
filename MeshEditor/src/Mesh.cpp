#include "Mesh.h"

#include "../../Interfaces/IRenderSystem.h"

#include <limits>
#include <vector>
#include <set>
#include <cstdlib>
#include <iostream>


Mesh::Mesh(const half_edge::HalfEdgeTable& halfEdgeTable) : table(halfEdgeTable)
{
	const auto& vertices = table.getVertices();

	if (vertices.empty())
	{
		m_bbox.min = glm::vec3(0.0f);
		m_bbox.max = glm::vec3(0.0f);
		return;
	}

	glm::vec3 minBounds(std::numeric_limits<float>::infinity());
	glm::vec3 maxBounds(-std::numeric_limits<float>::infinity());

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		const glm::vec3& point = table.getPoint(half_edge::VertexHandle{ static_cast<int64_t>(i) });
		minBounds = glm::min(minBounds, point);
		maxBounds = glm::max(maxBounds, point);
	}

	m_bbox.min = minBounds;
	m_bbox.max = maxBounds;
}

void Mesh::render(IRenderSystem& rs)
{
	//std::lock_guard<std::mutex> lock(renderMutex);

	const auto& faces = table.getFaces();
	std::vector<Vertex> triangle;
	triangle.reserve(3);

	for (size_t fi = 0; fi < faces.size(); ++fi)
	{
		const half_edge::FaceHandle fh{ static_cast<int64_t>(fi) };
		const half_edge::HalfEdgeHandle start = table.deref(fh).heh;

		if (start.index == -1)
		{
			continue;
		}
		
		half_edge::HalfEdgeHandle he = start;

		std::vector<glm::vec3> positions;
		positions.reserve(4);

		size_t maxIterations = 100;
		size_t iterations = 0;

		do
		{
			const half_edge::VertexHandle vh = table.destVertex(he);
			positions.push_back(table.getPoint(vh));
			he = table.next(he);

			if (he.index == -1 || ++iterations > maxIterations)
			{
				break;
			}
		} while (he.index != start.index);

		if (positions.size() < 3)
		{
			continue;
		}

		const glm::vec3 n = glm::cross(positions[1] - positions[0], positions[2] - positions[0]);
		const float nLen = glm::length(n);

		constexpr float epsilon = 1e-6f;
		const glm::vec3 normal = (nLen > epsilon) ? (n / nLen) : glm::vec3(0.0f);

		const glm::vec3 color(1.0f);

		for (size_t i = 1; i + 1 < positions.size(); ++i)
		{
			triangle.clear();
			triangle.push_back({ positions[0], normal, color });
			triangle.push_back({ positions[i], normal, color });
			triangle.push_back({ positions[i + 1], normal, color });

			rs.renderTriangleSoup(triangle);
		}
	}
}

void Mesh::applyTransformation(half_edge::FaceHandle fh, const glm::mat4& trf)
{
	//std::lock_guard<std::mutex> lock(renderMutex);

	const half_edge::HalfEdgeHandle start = table.deref(fh).heh;
	half_edge::HalfEdgeHandle he = start;

	size_t maxIterations = 100;
	size_t iterations = 0;

	do
	{
		const half_edge::VertexHandle vh = table.destVertex(he);
		glm::vec4 hp(table.getPoint(vh), 1.0f);
		hp = trf * hp;
		table.setPoint(vh, glm::vec3(hp));
		he = table.next(he);

		if (he.index == -1 || ++iterations > maxIterations)
		{
			break;
		}
	} while (he.index != start.index);

	const Mesh tmp(table);
	m_bbox = tmp.m_bbox;
}

const half_edge::HalfEdgeTable& Mesh::getHalfEdgeTable() const
{
	return table;
}

half_edge::HalfEdgeTable& Mesh::getHalfEdgeTable()
{
	return table;
}

const bbox& Mesh::getBoundingBox() const
{
	return m_bbox;
}

void Mesh::deleteFace(half_edge::FaceHandle fh)
{
	//std::lock_guard<std::mutex> lock(renderMutex);

	holes.clear();
	holeColors.clear();

	table.deleteFace(fh);

	const Mesh tmp(table);
	m_bbox = tmp.m_bbox;
}

void Mesh::renderEdges(IRenderSystem& rs)
{
	//std::lock_guard<std::mutex> lock(renderMutex);

	const auto& faces = table.getFaces();
	std::vector<Vertex> lineVertices;
	lineVertices.reserve(2);

	const glm::vec3 blackColor(0.0f, 0.0f, 0.0f);
	const glm::vec3 dummyNormal(0.0f, 0.0f, 1.0f);

	for (size_t fi = 0; fi < faces.size(); ++fi)
	{
		const half_edge::FaceHandle fh{ static_cast<int64_t>(fi) };
		const half_edge::HalfEdgeHandle start = table.deref(fh).heh;
		
		if (start.index == -1)
		{
			continue;
		}
		half_edge::HalfEdgeHandle he = start;

		size_t maxIterations = 100;
		size_t iterations = 0;

		do
		{
			const half_edge::HalfEdgeHandle twinHe = table.twin(he);
			
			if (he.index < twinHe.index || twinHe.index == -1)
			{
				const glm::vec3 startPoint = table.getStartPoint(he);
				const glm::vec3 endPoint = table.getEndPoint(he);

				lineVertices.clear();
				lineVertices.push_back({ startPoint, dummyNormal, blackColor });
				lineVertices.push_back({ endPoint, dummyNormal, blackColor });

				rs.renderLines(lineVertices);
			}

			he = table.next(he);

			if (he.index == -1 || ++iterations > maxIterations)
			{
				break;
			}
		} while (he.index != start.index);
	}
}

void Mesh::colorHoles()
{
	//std::lock_guard<std::mutex> lock(renderMutex);

	holes.clear();
	holeColors.clear();

	std::set<int64_t> visitedBoundaryEdges;

	const auto& vertices = table.getVertices();
	const auto& faces = table.getFaces();

	std::vector<half_edge::HalfEdgeHandle> allHalfEdges;

	for (size_t fi = 0; fi < faces.size(); ++fi)
	{
		const half_edge::FaceHandle fh{ static_cast<int64_t>(fi) };
		const half_edge::HalfEdgeHandle start = table.deref(fh).heh;

		if (start.index == -1)
			continue;

		half_edge::HalfEdgeHandle he = start;
		size_t maxIterations = 100;
		size_t iterations = 0;

			do
			{
				if (he.index >= 0)
				{
					allHalfEdges.push_back(he);

					half_edge::HalfEdgeHandle twinHe = table.twin(he);
					if (twinHe.index != -1 && table.deref(twinHe).fh.index == -1)
					{
						allHalfEdges.push_back(twinHe);
					}
				}

				he = table.next(he);

			if (he.index == -1 || ++iterations > maxIterations)
				break;

		} while (he.index != start.index);
	}

	for (const half_edge::HalfEdgeHandle& heh : allHalfEdges)
	{
		if (table.deref(heh).fh.index != -1)
			continue;

		if (visitedBoundaryEdges.find(heh.index) != visitedBoundaryEdges.end())
			continue;

		std::vector<half_edge::HalfEdgeHandle> hole;
		half_edge::HalfEdgeHandle currentHe = heh;
		half_edge::HalfEdgeHandle startHe = heh;

		size_t maxIterations = 10000;
		size_t iterations = 0;

		do
		{
			hole.push_back(currentHe);
			visitedBoundaryEdges.insert(currentHe.index);

			currentHe = table.next(currentHe);

			if (currentHe.index == -1 || ++iterations > maxIterations)
			{
				break;
			}
		} while (currentHe.index != startHe.index);

		if (!hole.empty() && currentHe.index == startHe.index)
		{
			holes.push_back(hole);

			const float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			const float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			const float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			holeColors.push_back(glm::vec3(r, g, b));

			std::cout << "Found hole #" << holes.size() << " with " << hole.size() << " boundary edges" << std::endl;
		}
	}

	std::cout << "Total holes found: " << holes.size() << std::endl;
}

void Mesh::renderColoredHoles(IRenderSystem& rs)
{
	//std::lock_guard<std::mutex> lock(renderMutex);

	std::vector<Vertex> lineVertices;
	lineVertices.reserve(2);

	const glm::vec3 dummyNormal(0.0f, 0.0f, 1.0f);

	for (size_t holeIdx = 0; holeIdx < holes.size(); ++holeIdx)
	{
		const glm::vec3& color = holeColors[holeIdx];
		const std::vector<half_edge::HalfEdgeHandle>& hole = holes[holeIdx];

		for (const half_edge::HalfEdgeHandle& he : hole)
		{
			if (he.index < 0)
				continue;

			try
			{
				const glm::vec3 startPoint = table.getStartPoint(he);
				const glm::vec3 endPoint = table.getEndPoint(he);

				lineVertices.clear();
				lineVertices.push_back({ startPoint, dummyNormal, color });
				lineVertices.push_back({ endPoint, dummyNormal, color });

				rs.renderLines(lineVertices);
			}
			catch (const std::exception& e)
			{
				std::cout << "Error rendering hole edge: " << e.what() << " (half-edge index: " << he.index << ")" << std::endl;
			}
			catch (...)
			{
				std::cout << "Unknown error rendering hole edge (half-edge index: " << he.index << ")" << std::endl;
			}
		}
	}
}

void Mesh::paintBoundaryFaces()
{
	//std::lock_guard<std::mutex> lock(renderMutex);

	boundaryFaces.clear();

	const auto& faces = table.getFaces();

	for (size_t fi = 0; fi < faces.size(); ++fi)
	{
		const half_edge::FaceHandle fh{ static_cast<int64_t>(fi) };
		const half_edge::HalfEdgeHandle start = table.deref(fh).heh;

		if (start.index == -1)
			continue;

		half_edge::HalfEdgeHandle he = start;
		bool hasBoundaryEdge = false;

		size_t maxIterations = 100;
		size_t iterations = 0;

		do
		{
			half_edge::HalfEdgeHandle twinHe = table.twin(he);

			if (twinHe.index == -1 || table.deref(twinHe).fh.index == -1)
			{
				hasBoundaryEdge = true;
				break;
			}

			he = table.next(he);

			if (he.index == -1 || ++iterations > maxIterations)
				break;

		} while (he.index != start.index);

		if (hasBoundaryEdge)
		{
			boundaryFaces.push_back(fh);
		}
	}

	std::cout << "Found " << boundaryFaces.size() << " boundary faces" << std::endl;
}

void Mesh::renderBoundaryFaces(IRenderSystem& rs)
{
	//std::lock_guard<std::mutex> lock(renderMutex);

	if (boundaryFaces.empty())
		return;

	const glm::vec3 redColor(1.0f, 0.0f, 0.0f); 
	const glm::vec3 dummyNormal(0.0f, 0.0f, 1.0f);

	std::vector<Vertex> triangles;

	for (const half_edge::FaceHandle& fh : boundaryFaces)
	{
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

		const glm::vec3 n = glm::cross(positions[1] - positions[0], positions[2] - positions[0]);
		const float nLen = glm::length(n);
		constexpr float epsilon = 1e-6f;
		const glm::vec3 normal = (nLen > epsilon) ? (n / nLen) : glm::vec3(0.0f);

		for (size_t i = 1; i + 1 < positions.size(); ++i)
		{
			triangles.push_back({ positions[0], normal, redColor });
			triangles.push_back({ positions[i], normal, redColor });
			triangles.push_back({ positions[i + 1], normal, redColor });
		}
	}

	if (!triangles.empty())
	{
		rs.renderTriangleSoup(triangles);
	}
}

void Mesh::clearBoundaryFaces()
{
	//std::lock_guard<std::mutex> lock(renderMutex);
	boundaryFaces.clear();
	std::cout << "Boundary faces cleared" << std::endl;
}

void Mesh::lockForRendering()
{
	renderMutex.lock();
}

void Mesh::unlockForRendering()
{
	renderMutex.unlock();
}

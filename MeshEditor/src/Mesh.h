#pragma once
#include "../../HalfEdge/src/HalfEdge.h" 
#include <glm/glm.hpp>
#include <mutex>

class IRenderSystem;

/**
 * @brief Axis-aligned bounding box.
 *
 * Represents a 3D bounding box aligned with the coordinate axes.
 * The bounding box encloses all vertices of the mesh.
 */
struct bbox
{
	glm::vec3 min;
	glm::vec3 max;
};

/**
 * @brief Polygonal mesh based on a half-edge data structure.
 *
 * The Mesh class owns a HalfEdgeTable and provides functionality
 * for rendering, geometric modification, and spatial queries such
 * as computing an axis-aligned bounding box.
 */
class Mesh
{
public:
	/**
	 * @brief Polygonal mesh based on a half-edge data structure.
	 *
	 * The Mesh class owns a HalfEdgeTable and provides functionality
	 * for rendering, geometric modification, and spatial queries such
	 * as computing an axis-aligned bounding box.
	 */
	Mesh(const half_edge::HalfEdgeTable& halfEdgeTable);

	/**
	 * @brief Renders the mesh using the given render system.
	 *
	 * Faces are triangulated into a triangle soup and submitted
	 * to the render system. Per-face normals are computed from
	 * vertex positions.
	 *
	 * @param rs Rendering backend used to draw the mesh.
	 */
	void render(IRenderSystem& rs);

	/**
	 * @brief Returns the axis-aligned bounding box of the mesh.
	 *
	 * @return Constant reference to the bounding box.
	 */
	const bbox& getBoundingBox() const;

	/**
	 * @brief Applies a transformation to all vertices of a face.
	 *
	 * Each vertex belonging to the specified face is transformed
	 * by the given 4x4 matrix. After the transformation, the mesh
	 * bounding box is recomputed.
	 *
	 * @param fh Handle to the face to be transformed.
	 * @param trf Transformation matrix (homogeneous coordinates).
	 */
	void applyTransformation(half_edge::FaceHandle fh, const glm::mat4& trf);

	/**
	 * @brief Deletes a face from the mesh.
	 *
	 * Removes the specified face from the half-edge structure and
	 * recomputes the mesh bounding box.
	 *
	 * @param fh Handle to the face to delete.
	 */
	void deleteFace(half_edge::FaceHandle fh);

	/**
	 * @brief Deletes a face from the mesh.
	 *
	 * Removes the specified face from the half-edge structure and
	 * recomputes the mesh bounding box.
	 *
	 * @param fh Handle to the face to delete.
	 */
	const half_edge::HalfEdgeTable& getHalfEdgeTable() const;
	half_edge::HalfEdgeTable& getHalfEdgeTable();

	/**
	 * @brief Renders the edges of the mesh.
	 *
	 * Iterates over the half-edge structure and submits each unique
	 * edge as a line segment to the render system, producing a
	 * wireframe representation of the mesh.
	 *
	 * @param rs Rendering backend used to draw the mesh edges.
	 */
	void renderEdges(IRenderSystem& rs);

	/**
	 * @brief Detects and colors holes in the mesh.
	 *
	 * Identifies all boundary loops (holes) in the mesh and assigns
	 * a random color to each hole for visualization purposes.
	 */
	void colorHoles();

	/**
	 * @brief Renders colored holes in the mesh.
	 *
	 * Renders each detected hole as colored lines using the colors
	 * assigned by colorHoles().
	 *
	 * @param rs Rendering backend used to draw the colored holes.
	 */
	void renderColoredHoles(IRenderSystem& rs);

	void paintBoundaryFaces();
	void renderBoundaryFaces(IRenderSystem& rs);
	void clearBoundaryFaces();

	void lockForRendering();
	void unlockForRendering();

private:
	/** Half-edge representation of the mesh topology and geometry. */
	half_edge::HalfEdgeTable table;
	/** Axis-aligned bounding box enclosing all vertices of the mesh. */
	bbox m_bbox;
	/** List of holes, each hole is a list of half-edge handles forming a boundary loop. */
	std::vector<std::vector<half_edge::HalfEdgeHandle>> holes;
	/** Colors assigned to each hole. */
	std::vector<glm::vec3> holeColors;

	std::vector<half_edge::FaceHandle> boundaryFaces;

	std::mutex renderMutex;
};

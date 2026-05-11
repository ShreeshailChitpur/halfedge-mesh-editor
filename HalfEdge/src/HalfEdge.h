#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <set>

namespace half_edge
{
	struct HalfEdgeHandle { int64_t index = -1; };
	struct VertexHandle { int64_t index = -1; };
	struct FaceHandle { int64_t index = -1; };

	struct HalfEdge
	{
		//The face it belongs to, is invalid (== -1) if a boundary half-edge 
		FaceHandle fh;
		//The vertex it points to is always valid 
		VertexHandle dst;
		HalfEdgeHandle twin;
		//The next HalfEdge in the CCW order is always valid 
		HalfEdgeHandle next;
		//The previous HalfEdge in the CCW order can be stored for the optimization purposes.For the triangle meshes prev = next->next->next
		HalfEdgeHandle prev;
	};

	struct Face
	{
		//One of the HalfEdges belonging to the Face, always valid 
		HalfEdgeHandle heh;
	};

	struct HEVertex
	{
		//An outgoing HalfEdge from this vertex. It is == -1 if the vertex is isolated 
		HalfEdgeHandle heh;
	};

	class HalfEdgeTable
	{
	public:
		//Adds vertex 
		VertexHandle addVertex(glm::vec3 position);

		//Adds triangulated face
		FaceHandle addFace(VertexHandle vh0, VertexHandle vh1, VertexHandle vh2);

		//Adds quad face 
		FaceHandle addFace(VertexHandle vh0, VertexHandle vh1, VertexHandle vh2, VertexHandle vh3);

		//Builds twins for half-edges. This function must be called in the end 
		void connectTwins();

		//Deletes face and re-link all vertices, half-edges, faces.  
		void deleteFace(FaceHandle fh);

	public:
		//For a given half-edge returns previous linked half-edge 
		HalfEdgeHandle prev(HalfEdgeHandle heh) const;

		//For a given half-edge returns next linked half-edge 
		HalfEdgeHandle next(HalfEdgeHandle heh) const;

		//For a given half-edge returns twin half-edge 
		HalfEdgeHandle twin(HalfEdgeHandle heh) const;

		//For a given half-edge returns end vertex 
		VertexHandle destVertex(HalfEdgeHandle heh) const;

		//For a given half-edge returns start vertex 
		VertexHandle sourceVertex(HalfEdgeHandle heh) const;

		//For a given half-edge handle returns half-edge 
		HalfEdge& deref(HalfEdgeHandle vh);

		const HalfEdge& deref(HalfEdgeHandle vh) const;

		//For a given half-edge returns half-edge handle 
		HalfEdgeHandle handle(const HalfEdge& v) const;

		HEVertex& deref(VertexHandle vh);
		const HEVertex& deref(VertexHandle vh) const;
		VertexHandle handle(const HEVertex& v) const;

		Face& deref(FaceHandle fh);
		const Face& deref(FaceHandle fh) const;
		FaceHandle handle(const Face& f) const;

		//For a given vertex handle set/get point 
		const glm::vec3& getPoint(VertexHandle handle) const;
		void setPoint(VertexHandle handle, glm::vec3 data);

		//For a given half-edge handle set/get start point (vertex) 
		const glm::vec3& getStartPoint(HalfEdgeHandle handle) const;
		void setStartPoint(HalfEdgeHandle handle, glm::vec3 data);

		//For a given half-edge handle set/get end point (vertex) 
		const glm::vec3& getEndPoint(HalfEdgeHandle handle) const;
		void setEndPoint(HalfEdgeHandle handle, glm::vec3 data);

		const std::vector<HEVertex>& getVertices() const;
		const std::vector<Face>& getFaces() const;

	private:
		void collectFaceData(FaceHandle fh, std::vector<HalfEdgeHandle>& faceHalfEdges, std::vector<HalfEdgeHandle>& boundaryHalfEdges, std::set<int64_t>& vertexSet);
		void relinkBoundaryEdges(const std::vector<HalfEdgeHandle>& boundaryHalfEdges, std::set<int64_t>& markedForDeletion);
		void detachFaceTwins(const std::vector<HalfEdgeHandle>& faceHalfEdges);
		void updateVerticesAfterDeletion(const std::set<int64_t>& vertexSet, const std::vector<HalfEdgeHandle>& faceHalfEdges);
		void compactAfterDeletion(FaceHandle fh, const std::vector<HalfEdgeHandle>& faceHalfEdges, const std::set<int64_t>& markedForDeletion);
		void removeIsolatedVertices();

	private:
		std::vector<HalfEdge> halfEdges;
		std::vector<HEVertex> vertices;
		std::vector<Face> faces;
		std::vector<glm::vec3> points;
	};
}

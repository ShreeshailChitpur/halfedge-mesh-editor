#include "HalfEdge.h"

#include <map>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <limits>
#include <unordered_set>

namespace half_edge
{
	VertexHandle HalfEdgeTable::addVertex(glm::vec3 position)
	{
		constexpr float eps = 1e-6f;

		for (int64_t i = 0; i < static_cast<int64_t>(points.size()); ++i)
		{
			if (glm::length(points[i] - position) < eps)
			{
				return VertexHandle{ i };
			}
		}

		HEVertex v;
		v.heh.index = -1;
		vertices.push_back(v);
		points.push_back(position);
		return VertexHandle{ static_cast<int64_t>(vertices.size() - 1) };
	}

	FaceHandle HalfEdgeTable::addFace(VertexHandle vh0, VertexHandle vh1, VertexHandle vh2)
	{
		const int64_t baseIndex = static_cast<int64_t>(halfEdges.size());

		faces.push_back({ static_cast<int64_t>(halfEdges.size()) });
		FaceHandle fh = { static_cast<int64_t>(faces.size() - 1) };

		HalfEdge he0;
		he0.dst = vh1;
		he0.next.index = baseIndex + 1;
		he0.prev.index = baseIndex + 2;

		HalfEdge he1;
		he1.dst = vh2;
		he1.next.index = baseIndex + 2;
		he1.prev.index = baseIndex + 0;

		HalfEdge he2;
		he2.dst = vh0;
		he2.next.index = baseIndex + 0;
		he2.prev.index = baseIndex + 1;

		he0.fh = fh;
		he1.fh = fh;
		he2.fh = fh;

		halfEdges.push_back(he0);
		halfEdges.push_back(he1);
		halfEdges.push_back(he2);

		if (deref(vh0).heh.index == -1)
		{
			deref(vh0).heh.index = baseIndex + 0;
		}
		if (deref(vh1).heh.index == -1)
		{
			deref(vh1).heh.index = baseIndex + 1;
		}
		if (deref(vh2).heh.index == -1)
		{
			deref(vh2).heh.index = baseIndex + 2;
		}

		return fh;
	}

	FaceHandle HalfEdgeTable::addFace(VertexHandle vh0, VertexHandle vh1, VertexHandle vh2, VertexHandle vh3)
	{
		const int64_t baseIndex = static_cast<int64_t>(halfEdges.size());

		faces.push_back({ static_cast<int64_t>(halfEdges.size()) });
		FaceHandle fh = { static_cast<int64_t>(faces.size() - 1) };

		HalfEdge he0;
		he0.dst = vh1;
		he0.fh = fh;
		he0.next.index = baseIndex + 1;
		he0.prev.index = baseIndex + 3;

		HalfEdge he1;
		he1.dst = vh2;
		he1.fh = fh;
		he1.next.index = baseIndex + 2;
		he1.prev.index = baseIndex + 0;

		HalfEdge he2;
		he2.dst = vh3;
		he2.fh = fh;
		he2.next.index = baseIndex + 3;
		he2.prev.index = baseIndex + 1;

		HalfEdge he3;
		he3.dst = vh0;
		he3.fh = fh;
		he3.next.index = baseIndex + 0;
		he3.prev.index = baseIndex + 2;

		halfEdges.push_back(he0);
		halfEdges.push_back(he1);
		halfEdges.push_back(he2);
		halfEdges.push_back(he3);

		if (deref(vh0).heh.index == -1)
		{
			deref(vh0).heh.index = baseIndex + 0;
		}
		if (deref(vh1).heh.index == -1)
		{
			deref(vh1).heh.index = baseIndex + 1;
		}
		if (deref(vh2).heh.index == -1)
		{
			deref(vh2).heh.index = baseIndex + 2;
		}
		if (deref(vh3).heh.index == -1)
		{
			deref(vh3).heh.index = baseIndex + 3;
		}

		return fh;
	}

	HalfEdge& HalfEdgeTable::deref(HalfEdgeHandle heh)
	{
		return halfEdges[heh.index];
	}

	const HalfEdge& HalfEdgeTable::deref(HalfEdgeHandle heh) const
	{
		return halfEdges[heh.index];
	}

	HEVertex& HalfEdgeTable::deref(VertexHandle vh)
	{
		return vertices[vh.index];
	}

	const HEVertex& HalfEdgeTable::deref(VertexHandle vh) const
	{
		return vertices[vh.index];
	}

	Face& HalfEdgeTable::deref(FaceHandle fh)
	{
		return faces[fh.index];
	}

	const Face& HalfEdgeTable::deref(FaceHandle fh) const
	{
		return faces[fh.index];
	}

	HalfEdgeHandle HalfEdgeTable::handle(const HalfEdge& he) const
	{
		return { static_cast<int>(&he - &halfEdges[0]) };
	}

	VertexHandle HalfEdgeTable::handle(const HEVertex& v) const
	{
		return { static_cast<int>(&v - &vertices[0]) };
	}

	FaceHandle HalfEdgeTable::handle(const Face& f) const
	{
		return { static_cast<int>(&f - &faces[0]) };
	}

	HalfEdgeHandle HalfEdgeTable::next(HalfEdgeHandle heh) const
	{
		return deref(heh).next;
	}

	HalfEdgeHandle HalfEdgeTable::prev(HalfEdgeHandle heh) const
	{
		return deref(heh).prev;
	}

	HalfEdgeHandle HalfEdgeTable::twin(HalfEdgeHandle heh) const
	{
		return deref(heh).twin;
	}

	VertexHandle HalfEdgeTable::destVertex(HalfEdgeHandle heh) const
	{
		return deref(heh).dst;
	}

	VertexHandle HalfEdgeTable::sourceVertex(HalfEdgeHandle heh) const
	{
		return destVertex(twin(heh));
	}

	const glm::vec3& HalfEdgeTable::getPoint(VertexHandle handle) const
	{
		return points[handle.index];
	}

	void HalfEdgeTable::setPoint(VertexHandle handle, glm::vec3 data)
	{
		points[handle.index] = data;
	}

	const glm::vec3& HalfEdgeTable::getStartPoint(HalfEdgeHandle handle) const
	{
		return getPoint(sourceVertex(handle));
	}

	void HalfEdgeTable::setStartPoint(HalfEdgeHandle handle, glm::vec3 data)
	{
		setPoint(sourceVertex(handle), data);
	}

	const glm::vec3& HalfEdgeTable::getEndPoint(HalfEdgeHandle handle) const
	{
		return getPoint(destVertex(handle));
	}

	void HalfEdgeTable::setEndPoint(HalfEdgeHandle handle, glm::vec3 data)
	{
		setPoint(destVertex(handle), data);
	}

	const std::vector<HEVertex>& HalfEdgeTable::getVertices() const
	{
		return vertices;
	}

	const std::vector<Face>& HalfEdgeTable::getFaces() const
	{
		return faces;
	}

	void HalfEdgeTable::connectTwins()
	{
		std::map<std::pair<int64_t, int64_t>, HalfEdgeHandle> edgeMap;

		for (int64_t i = 0; i < static_cast<int64_t>(halfEdges.size()); ++i)
		{
			HalfEdgeHandle heh{ i };
			VertexHandle src = destVertex(prev(heh));
			VertexHandle dst = destVertex(heh);
			auto it = edgeMap.find({ dst.index, src.index });
			if (it != edgeMap.end())
			{
				HalfEdgeHandle twinHeh = it->second;
				deref(heh).twin = twinHeh;
				deref(twinHeh).twin = heh;
				edgeMap.erase(it);
			}
			else
			{
				edgeMap.emplace(std::make_pair(src.index, dst.index), heh);
			}
		}

		std::vector<HalfEdgeHandle> boundaryHandles;

		for (const auto& entry : edgeMap)
		{
			HalfEdgeHandle interiorHeh = entry.second;
			VertexHandle src = destVertex(prev(interiorHeh));
			VertexHandle dst = destVertex(interiorHeh);

			HalfEdge boundaryHe;
			boundaryHe.dst = src;
			boundaryHe.fh.index = -1;
			boundaryHe.next.index = -1;
			boundaryHe.prev.index = -1;
			boundaryHe.twin = interiorHeh;
			halfEdges.emplace_back(HalfEdge{ boundaryHe });
			HalfEdgeHandle boundaryHeh{ static_cast<int64_t>(halfEdges.size() - 1) };
			deref(interiorHeh).twin = boundaryHeh;
			boundaryHandles.push_back(boundaryHeh);
		}

		std::unordered_map<int64_t, std::vector<HalfEdgeHandle>> outgoing;
		for (HalfEdgeHandle bheh : boundaryHandles)
		{
			outgoing[sourceVertex(bheh).index].push_back(bheh);
		}

		auto getInteriorFaceNormal = [this](HalfEdgeHandle boundaryHeh) -> glm::vec3
			{
				HalfEdgeHandle interiorHeh = twin(boundaryHeh);
				HalfEdgeHandle h0 = interiorHeh;
				HalfEdgeHandle h1 = next(h0);
				HalfEdgeHandle h2 = next(h1);

				glm::vec3 p0 = getPoint(destVertex(h0));
				glm::vec3 p1 = getPoint(destVertex(h1));
				glm::vec3 p2 = getPoint(destVertex(h2));

				glm::vec3 v1 = p1 - p0;
				glm::vec3 v2 = p2 - p0;
				glm::vec3 normal = glm::cross(v1, v2);

				if (glm::length(normal) > 1e-6f)
				{
					return normal / glm::length(normal);
				}

				return glm::vec3(0, 0, 1);
			};

		std::unordered_set<int64_t> visited;

		int loopCount = 0;
		for (HalfEdgeHandle start : boundaryHandles)
		{
			if (visited.count(start.index))
				continue;

			HalfEdgeHandle current = start;
			int stepCount = 0;

			do
			{
				visited.insert(current.index);

				VertexHandle srcVertex = sourceVertex(current);
				VertexHandle dstVertex = destVertex(current);

				auto& candidates = outgoing[dstVertex.index];

				if (candidates.empty())
				{
					break;
				}

				if (candidates.size() == 1)
				{
					HalfEdgeHandle nextEdge = candidates[0];
					deref(current).next = nextEdge;
					deref(nextEdge).prev = current;
					current = nextEdge;
					continue;
				}

				glm::vec3 refNormal = getInteriorFaceNormal(current);
				glm::vec3 pos_from = getPoint(srcVertex);
				glm::vec3 pos_to = getPoint(dstVertex);
				glm::vec3 incoming = glm::normalize(pos_to - pos_from);

				HalfEdgeHandle bestEdge{ -1 };
				float bestAngle = std::numeric_limits<float>::max();

				for (HalfEdgeHandle cand : candidates)
				{
					if (cand.index == current.index)
					{
						continue;
					}

					VertexHandle v_next = destVertex(cand);
					glm::vec3 pos_next = getPoint(v_next);
					glm::vec3 outgoing = glm::normalize(pos_next - pos_to);

					glm::vec3 crossProd = glm::cross(incoming, outgoing);
					float signedComponent = glm::dot(crossProd, refNormal);

					float cosAngle = glm::dot(incoming, outgoing);
					cosAngle = glm::clamp(cosAngle, -1.0f, 1.0f);
					float unsignedAngle = acos(cosAngle);

					float signedAngle = (signedComponent >= 0) ? unsignedAngle : -unsignedAngle;

					if (signedAngle > 1e-6f && signedAngle < bestAngle)
					{
						bestAngle = signedAngle;
						bestEdge = cand;
					}
				}

				if (bestEdge.index == -1)
				{
					float leastNegative = -std::numeric_limits<float>::max();

					for (HalfEdgeHandle cand : candidates)
					{
						if (cand.index == current.index)
							continue;

						VertexHandle v_next = destVertex(cand);
						glm::vec3 pos_next = getPoint(v_next);
						glm::vec3 outgoing = glm::normalize(pos_next - pos_to);

						glm::vec3 crossProd = glm::cross(incoming, outgoing);
						float signedComponent = glm::dot(crossProd, refNormal);

						float cosAngle = glm::dot(incoming, outgoing);
						cosAngle = glm::clamp(cosAngle, -1.0f, 1.0f);
						float unsignedAngle = acos(cosAngle);
						float signedAngle = (signedComponent >= 0) ? unsignedAngle : -unsignedAngle;


						if (signedAngle > leastNegative)
						{
							leastNegative = signedAngle;
							bestEdge = cand;
						}
					}
				}

				if (bestEdge.index != -1)
				{
					deref(current).next = bestEdge;
					deref(bestEdge).prev = current;
				}
				else
				{
					break;
				}

				current = bestEdge;

			} while (current.index != -1 && current.index != start.index && !visited.count(current.index));
		}

		for (int64_t i = 0; i < static_cast<int64_t>(vertices.size()); ++i)
		{
			VertexHandle vh{ i };
			HalfEdgeHandle start = deref(vh).heh;

			if (start.index == -1)
				continue;

			HalfEdgeHandle he = start;
			do
			{
				if (deref(he).fh.index == -1)
				{
					deref(vh).heh = he;
					break;
				}
				he = twin(prev(he));
			} while (he.index != start.index && he.index != -1);
		}
	}

	void HalfEdgeTable::collectFaceData(FaceHandle fh, std::vector<HalfEdgeHandle>& faceHalfEdges, std::vector<HalfEdgeHandle>& boundaryHalfEdges, std::set<int64_t>& vertexSet)
	{
		HalfEdgeHandle startHeh = deref(fh).heh;
		HalfEdgeHandle currentHeh = startHeh;

		do
		{
			faceHalfEdges.push_back(currentHeh);
			vertexSet.insert(destVertex(currentHeh).index);

			HalfEdgeHandle twinHeh = twin(currentHeh);
			if (twinHeh.index == -1)
			{
				boundaryHalfEdges.push_back(currentHeh);
			}

			currentHeh = next(currentHeh);
		} while (currentHeh.index != startHeh.index);
	}

	void HalfEdgeTable::relinkBoundaryEdges(const std::vector<HalfEdgeHandle>& boundaryHalfEdges, std::set<int64_t>& markedForDeletion)
	{
		for (HalfEdgeHandle heh : boundaryHalfEdges)
		{
			HalfEdgeHandle h0 = heh;
			VertexHandle v0 = deref(h0).dst;
			HalfEdgeHandle next0 = deref(h0).next;
			HalfEdgeHandle prev0 = deref(h0).prev;

			HalfEdgeHandle h1 = twin(heh);
			VertexHandle v1 = deref(h1).dst;
			HalfEdgeHandle next1 = deref(h1).next;
			HalfEdgeHandle prev1 = deref(h1).prev;

			deref(prev0).next = next1;
			deref(next1).prev = prev0;
			deref(prev1).next = next0;
			deref(next0).prev = prev1;

			markedForDeletion.insert(h0.index);
			markedForDeletion.insert(h1.index);

			if (deref(v0).heh.index == h1.index)
			{
				if (next0.index == h1.index)
				{
					markedForDeletion.insert(v0.index);
				}
				else
				{
					deref(v0).heh = next0;
				}
			}

			if (deref(v1).heh.index == h0.index)
			{
				if (next1.index == h0.index)
				{
					markedForDeletion.insert(v1.index);
				}
				else
				{
					deref(v1).heh = next1;
				}
			}
		}
	}

	void HalfEdgeTable::detachFaceTwins(const std::vector<HalfEdgeHandle>& faceHalfEdges)
	{
		for (HalfEdgeHandle heh : faceHalfEdges)
		{
			HalfEdgeHandle twinHeh = twin(heh);
			if (twinHeh.index != -1)
			{
				deref(twinHeh).twin.index = -1;
			}
		}
	}

	void HalfEdgeTable::updateVerticesAfterDeletion(const std::set<int64_t>& vertexSet, const std::vector<HalfEdgeHandle>& faceHalfEdges)
	{
		for (int64_t vhIndex : vertexSet)
		{
			VertexHandle vh{ vhIndex };
			HalfEdgeHandle vheh = deref(vh).heh;

			bool needsUpdate = false;
			for (HalfEdgeHandle heh : faceHalfEdges)
			{
				if (vheh.index == heh.index)
				{
					needsUpdate = true;
					break;
				}
			}

			if (needsUpdate)
			{
				bool foundBoundary = false;
				HalfEdgeHandle currentOutgoing = vheh;
				HalfEdgeHandle startOutgoing = currentOutgoing;

				do
				{
					HalfEdgeHandle twinHeh = twin(currentOutgoing);
					if (twinHeh.index != -1)
					{
						if (deref(twinHeh).fh.index == -1)
						{
							deref(vh).heh = twinHeh;
							foundBoundary = true;
							break;
						}
						currentOutgoing = next(twinHeh);
					}
					else
					{
						break;
					}
				} while (currentOutgoing.index != startOutgoing.index && currentOutgoing.index != -1);

				if (!foundBoundary)
				{
					deref(vh).heh.index = -1;
				}
			}
		}
	}

	void HalfEdgeTable::compactAfterDeletion(FaceHandle fh, const std::vector<HalfEdgeHandle>& faceHalfEdges, const std::set<int64_t>& markedForDeletion)
	{
		std::vector<int64_t> heIndices;
		for (HalfEdgeHandle heh : faceHalfEdges)
		{
			heIndices.push_back(heh.index);
		}

		for (int64_t idx : markedForDeletion)
		{
			heIndices.push_back(idx);
		}

		std::sort(heIndices.begin(), heIndices.end());
		heIndices.erase(std::unique(heIndices.begin(), heIndices.end()), heIndices.end());
		std::sort(heIndices.begin(), heIndices.end(), std::greater<int64_t>());

		int64_t oldSize = static_cast<int64_t>(halfEdges.size());
		std::set<int64_t> deleted(heIndices.begin(), heIndices.end());

		std::vector<int64_t> map(oldSize, -1);

		int64_t newIdx = 0;
		for (int64_t i = 0; i < oldSize; ++i)
		{
			if (deleted.count(i) == 0)
				map[i] = newIdx++;
		}

		for (int64_t idx : heIndices)
		{
			halfEdges.erase(halfEdges.begin() + idx);
		}

		faces.erase(faces.begin() + fh.index);

		for (int64_t i = 0; i < static_cast<int64_t>(halfEdges.size()); ++i)
		{
			HalfEdge& he = deref(HalfEdgeHandle{ i });

			if (he.fh.index > fh.index)
				he.fh.index--;
		}

		for (int64_t i = 0; i < static_cast<int64_t>(halfEdges.size()); ++i)
		{
			HalfEdge& he = deref(HalfEdgeHandle{ i });

			if (he.next.index != -1 && he.next.index < static_cast<int64_t>(map.size()))
				he.next.index = map[he.next.index];
			else if (he.next.index != -1)
				he.next.index = -1;

			if (he.prev.index != -1 && he.prev.index < static_cast<int64_t>(map.size()))
				he.prev.index = map[he.prev.index];
			else if (he.prev.index != -1)
				he.prev.index = -1;

			if (he.twin.index != -1 && he.twin.index < static_cast<int64_t>(map.size()))
				he.twin.index = map[he.twin.index];
			else if (he.twin.index != -1)
				he.twin.index = -1;
		}

		for (int64_t i = 0; i < static_cast<int64_t>(vertices.size()); ++i)
		{
			HEVertex& v = deref(VertexHandle{ i });

			if (v.heh.index != -1 && v.heh.index < static_cast<int64_t>(map.size()))
				v.heh.index = map[v.heh.index];
			else if (v.heh.index != -1)
				v.heh.index = -1;
		}

		for (int64_t i = 0; i < static_cast<int64_t>(faces.size()); ++i)
		{
			Face& f = deref(FaceHandle{ i });

			if (f.heh.index != -1 && f.heh.index < static_cast<int64_t>(map.size()))
				f.heh.index = map[f.heh.index];
			else if (f.heh.index != -1)
				f.heh.index = -1;
		}
	}

	void HalfEdgeTable::removeIsolatedVertices()
	{
		std::vector<int64_t> isolatedVertices;
		for (int64_t i = 0; i < static_cast<int64_t>(vertices.size()); ++i)
		{
			HEVertex& v = deref(VertexHandle{ i });
			if (v.heh.index == -1)
			{
				isolatedVertices.push_back(i);
			}
		}

		if (!isolatedVertices.empty())
		{
			std::sort(isolatedVertices.begin(), isolatedVertices.end(), std::greater<int64_t>());

			int64_t oldVertexSize = static_cast<int64_t>(vertices.size());
			std::set<int64_t> deletedVertices(isolatedVertices.begin(), isolatedVertices.end());

			std::vector<int64_t> vertexMap(oldVertexSize, -1);

			int64_t newVertexIdx = 0;
			for (int64_t i = 0; i < oldVertexSize; ++i)
			{
				if (deletedVertices.count(i) == 0)
					vertexMap[i] = newVertexIdx++;
			}

			for (int64_t idx : isolatedVertices)
			{
				vertices.erase(vertices.begin() + idx);
				points.erase(points.begin() + idx);
			}

			for (int64_t i = 0; i < static_cast<int64_t>(halfEdges.size()); ++i)
			{
				HalfEdge& he = deref(HalfEdgeHandle{ i });

				if (he.dst.index != -1 && he.dst.index < static_cast<int64_t>(vertexMap.size()))
					he.dst.index = vertexMap[he.dst.index];
				else if (he.dst.index != -1)
				{
					std::cout << "ERROR: Invalid vertex index " << he.dst.index << " after deletion (max: " << vertexMap.size() - 1 << ")" << std::endl;
					he.dst.index = -1;
				}
			}
		}
	}

	void HalfEdgeTable::deleteFace(FaceHandle fh)
	{
		std::vector<HalfEdgeHandle> faceHalfEdges;
		std::vector<HalfEdgeHandle> boundaryHalfEdges;
		std::set<int64_t> vertexSet;
		std::set<int64_t> markedForDeletion;

		collectFaceData(fh, faceHalfEdges, boundaryHalfEdges, vertexSet);

		relinkBoundaryEdges(boundaryHalfEdges, markedForDeletion);

		detachFaceTwins(faceHalfEdges);

		updateVerticesAfterDeletion(vertexSet, faceHalfEdges);

		compactAfterDeletion(fh, faceHalfEdges, markedForDeletion);

		removeIsolatedVertices();
	}
}

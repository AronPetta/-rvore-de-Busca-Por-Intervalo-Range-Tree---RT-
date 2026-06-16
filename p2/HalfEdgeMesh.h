#ifndef __HalfEdgeMesh_h
#define __HalfEdgeMesh_h

// =================================================================
// Author: Aron Petta Sarabia
// Last revision: 13/06/2026
// =================================================================

#include "TriangleMesh.h"
#include <vector>
#include <map>
#include <utility>
#include <functional>
#include <set>
#include <cmath>

namespace tcii::cg
{ // begin namespace tcii::cg

using index_t = unsigned;
constexpr index_t null_index = static_cast<index_t>(-1);

// Estruturas de Elementos Topológicos da Half-Edge
struct HE_HalfEdge
{
  index_t originVertex{null_index};
  index_t twin{null_index};
  index_t edge{null_index};
  index_t face{null_index}; // Permanece null_index se for uma borda externa (contorno)
  index_t next{null_index};
  index_t prev{null_index};
};

struct HE_Vertex
{
  index_t id{null_index};
  index_t halfEdge{null_index}; // Uma das semi-arestas que saem deste vértice
  Vec3f position{};
};

struct HE_Edge
{
  index_t id{null_index};
  index_t halfEdge{null_index}; // Aponta para uma das duas semi-arestas gêmeas associadas
};

struct HE_Face
{
  index_t id{null_index};
  index_t halfEdge{null_index}; // Uma das semi-arestas que delimitam esta face
};

struct HE_Boundary
{
  index_t id{null_index};
  index_t halfEdge{null_index}; // Semi-aresta virtual externa que percorre o contorno de borda
};

class HalfEdgeMesh
{
public:
  std::vector<HE_Vertex>   vertices;
  std::vector<HE_HalfEdge> halfEdges;
  std::vector<HE_Edge>     edges;
  std::vector<HE_Face>     faces;
  std::vector<HE_Boundary> boundaries;

  // Construtor que lê e converte a TriangleMesh do professor
  HalfEdgeMesh(const TriangleMesh& mesh)
  {
    buildFromTriangleMesh(mesh);
  }

  // Objetivo A2: Iteradores Básicos utilizando os containers contíguos
  auto vertexIter()   { return vertices.begin(); }
  auto edgeIter()     { return edges.begin(); }
  auto faceIter()     { return faces.begin(); }
  auto boundaryIter() { return boundaries.begin(); }

  // Objetivo A2: Processamento de Vizinhança - Arestas Incidentes a um Vértice
  void foreachIncidentEdge(index_t vertexId, std::function<void(index_t)> func)
  {
    index_t startHE = vertices[vertexId].halfEdge;
    if (startHE == null_index) return;

    index_t currHE = startHE;
    do {
      func(halfEdges[currHE].edge);
      // Rotaciona no sentido anti-horário em torno do vértice de origem
      index_t twin = halfEdges[currHE].twin;
      if (twin == null_index) break;
      currHE = halfEdges[twin].next;
    } while (currHE != startHE && currHE != null_index);
  }

  // Objetivo A2: Processamento de Vizinhança - Consulta Avançada por k-Anéis de Faces
  void foreachVertexKRingFaces(index_t vertexId, size_t k, std::function<void(index_t)> func)
  {
    if (k == 0) return;

    std::set<index_t> currentRing;
    std::set<index_t> visited;

    // Coleta o 1-Anel Inicial de faces adjacentes ao vértice
    index_t startHE = vertices[vertexId].halfEdge;
    if (startHE == null_index) return;
    index_t currHE = startHE;
    do {
      if (halfEdges[currHE].face != null_index)
        currentRing.insert(halfEdges[currHE].face);
        
      index_t twin = halfEdges[currHE].twin;
      if (twin == null_index) break;
      currHE = halfEdges[twin].next;
    } while (currHE != startHE && currHE != null_index);

    visited = currentRing;

    // Expandir indutivamente até atingir a profundidade do k-Anel pedido
    for (size_t ring = 1; ring < k; ++ring) {
      std::set<index_t> nextRing;
      for (index_t fId : currentRing) {
        index_t he = faces[fId].halfEdge;
        // Varre as 3 semi-arestas vizinhas da face atual
        for (int i = 0; i < 3; ++i) {
          index_t twinHE = halfEdges[he].twin;
          if (twinHE != null_index) {
            index_t adjFace = halfEdges[twinHE].face;
            if (adjFace != null_index && visited.find(adjFace) == visited.end()) {
              nextRing.insert(adjFace);
              visited.insert(adjFace);
            }
          }
          he = halfEdges[he].next;
        }
      }
      currentRing = std::move(nextRing);
    }

    // Executa a função callback do usuário em cada face coletada no anel expandido
    for (index_t fId : visited) {
      func(fId);
    }
  }

private:
  void buildFromTriangleMesh(const TriangleMesh& mesh)
  {
    auto& data = mesh.data();
    size_t nv = data.vertexCount();
    size_t nt = data.triangleCount();

    // Reserva de memória para evitar realocações dinâmicas custosas
    vertices.reserve(nv);
    halfEdges.reserve(nt * 3);
    edges.reserve(nt * 2);
    faces.reserve(nt);

    // 1. Instanciação dos Vértices copiando as posições originais
    for (index_t i = 0; i < nv; ++i) {
      vertices.push_back({i, null_index, data.vertex(i)});
    }

    // Mapeamento auxiliar: chave única para par de vértices não ordenados (vMin, vMax) -> Índice da Semi-Aresta
    auto makeKey = [](index_t v1, index_t v2) {
      return v1 < v2 ? std::make_pair(v1, v2) : std::make_pair(v2, v1);
    };
    std::map<std::pair<index_t, index_t>, index_t> edgeMap;

    // 2. Construção das Faces e Semi-Arestas internas
    for (index_t t = 0; t < nt; ++t) {
      auto& triangle = data.triangle(t);
      index_t fId = static_cast<index_t>(faces.size());
      faces.push_back({fId, null_index});

      index_t heIdx[3];
      for (int i = 0; i < 3; ++i) {
        heIdx[i] = static_cast<index_t>(halfEdges.size());
        halfEdges.push_back({});
      }

      faces[fId].halfEdge = heIdx[0];

      for (int i = 0; i < 3; ++i) {
        index_t vStart = triangle[i];
        index_t vEnd = triangle[(i + 1) % 3];

        HE_HalfEdge& he = halfEdges[heIdx[i]];
        he.originVertex = vStart;
        he.face = fId;
        he.next = heIdx[(i + 1) % 3];
        he.prev = heIdx[(i + 2) % 3];

        // Vincula temporariamente uma semi-aresta de saída ao vértice de origem
        vertices[vStart].halfEdge = heIdx[i];

        // Costura inteligente de arestas físicas (Edges) e atribuição de gêmeas (`twin`)
        auto key = makeKey(vStart, vEnd);
        auto it = edgeMap.find(key);
        if (it == edgeMap.end()) {
          index_t eId = static_cast<index_t>(edges.size());
          edges.push_back({eId, heIdx[i]});
          he.edge = eId;
          edgeMap[key] = heIdx[i];
        } else {
          index_t partnerIdx = it->second;
          he.edge = halfEdges[partnerIdx].edge;
          he.twin = partnerIdx;
          halfEdges[partnerIdx].twin = heIdx[i];
        }
      }
    }

    // 3. Resolução automática de Contornos (Boundaries) para malhas abertas
    index_t bIdCounter = 0;
    size_t initialHECount = halfEdges.size();
    for (size_t i = 0; i < initialHECount; ++i) {
      if (halfEdges[i].twin == null_index) {
        index_t boundaryHE = static_cast<index_t>(halfEdges.size());
        halfEdges.push_back({});
        
        index_t vStart = halfEdges[halfEdges[i].next].originVertex;
        
        halfEdges[boundaryHE].originVertex = vStart;
        halfEdges[boundaryHE].twin = static_cast<index_t>(i);
        halfEdges[i].twin = boundaryHE;
        halfEdges[boundaryHE].face = null_index; // Identifica que é uma borda de contorno virtual
        
        index_t bId = bIdCounter++;
        boundaries.push_back({bId, boundaryHE});
      }
    }
  }
};

} // end namespace tcii::cg

#endif // __HalfEdgeMesh_h
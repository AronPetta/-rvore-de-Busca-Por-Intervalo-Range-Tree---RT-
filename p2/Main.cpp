#include <iostream>
#include "HalfEdgeMesh.h"
#include "MeshDecoration.h"

using namespace tcii::cg;

// Enumeração explícita para indexar os 4 componentes dentro da tupla DecorationSet
enum { VD, ED, FD, BD };

struct RGBA { float r, g, b, a; };

// Estágio 1: Inicializa propriedades cromáticas nas Faces (Índice 2 / FD)
auto stage1_init(const HalfEdgeMesh& mesh)
{
  using VA = void;
  using EA = void;
  using TA = ElementSoA<RGBA>; // Apenas as faces ganham atributos aqui
  using BA = void;
  using MD = MeshDecoration<VA, EA, TA, BA>;

  auto md = MD::New(mesh);
  std::cout << "[Stage 1] Alocado. Elementos de Face no SoA: " << md->mesh()->faces.size() << "\n";

  // Preenche todas as faces com uma cor opaca padrão
  for (size_t i = 0; i < md->mesh()->faces.size(); ++i) {
    attributes<FD>(*md).set(static_cast<index_t>(i), RGBA{0.2f, 0.4f, 0.8f, 1.0f});
  }
  return md;
}

// Estágio 2: Preserva dados anteriores e adiciona métricas físicas nas Arestas (Índice 1 / ED)
template <typename PreviousMD>
auto stage2_metrics(PreviousMD* previous)
{
  using VA = void;
  using EA = ElementSoA<float>; // Adiciona campo de comprimento/peso geométrico na Aresta
  using TA = typename PreviousMD::TA; // Propaga de forma transparente o tipo do estágio 1
  using BA = void;
  using MD = MeshDecoration<VA, EA, TA, BA>;

  auto md = MD::New(previous); // Transfere a posse via std::move internamente
  
  // Computa comprimentos fictícios ou distâncias com base nos vértices da aresta half-edge
  for (size_t i = 0; i < md->mesh()->edges.size(); ++i) {
    attributes<ED>(*md).set(static_cast<index_t>(i), 1.5f * static_cast<float>(i + 1));
  }
  std::cout << "[Stage 2] Metricas de arestas acopladas ao conjunto decorativo.\n";
  return md;
}

// Estágio 3: Finalização da malha injetando dados nos Vértices (VD) e Contornos de Borda (BD)
template <typename PreviousMD>
auto stage3_finalize(PreviousMD* previous)
{
  using VA = ElementSoA<int>;    // ID de validação topológica nos vértices
  using EA = typename PreviousMD::EA;
  using TA = typename PreviousMD::TA;
  using BA = ElementSoA<bool>;   // Flag de contorno aberto/fechado nas bordas externas
  using MD = MeshDecoration<VA, EA, TA, BA>;

  auto md = MD::New(previous);
  
  for (size_t i = 0; i < md->mesh()->vertices.size(); ++i) {
    attributes<VD>(*md).set(static_cast<index_t>(i), 1000 + static_cast<int>(i));
  }
  
  for (size_t i = 0; i < md->mesh()->boundaries.size(); ++i) {
    attributes<BD>(*md).set(static_cast<index_t>(i), true);
  }

  std::cout << "[Stage 3] Pipeline concluido com sucesso. Todos os 4 slots avaliados.\n";
  return md;
}

int main()
{
  std::cout << "==================================================\n";
  std::cout << "  Sistema de Decoracoes Variadicas da Malha (P2)  \n";
  std::cout << "  Autor: Aron Petta Sarabia                       \n";
  std::cout << "==================================================\n";

  // 1. Carregamento de um ficheiro OBJ real através do OBJReader do professor
  // Substitua pelo nome de um ficheiro .obj que tenha na sua pasta de testes
  const char* filename = "f-16.obj"; 
  std::cout << "[Mesh] A carregar o ficheiro: " << filename << "...\n";
  
  ObjectPtr<TriangleMesh> tMesh = readOBJ(filename);
  
  if (tMesh == nullptr) {
    std::cout << "[Erro] Nao foi possivel abrir o ficheiro " << filename << ".\n";
    std::cout << "A executar teste com malha mock alternativa...\n";
    
    // Fallback caso o ficheiro não exista na pasta ainda
    TriangleMesh::Data mockData(4, 2);
    mockData.vertex(0) = {0.0f, 0.0f, 0.0f};
    mockData.vertex(1) = {1.0f, 0.0f, 0.0f};
    mockData.vertex(2) = {0.0f, 1.0f, 0.0f};
    mockData.vertex(3) = {1.0f, 1.0f, 0.0f};
    mockData.triangle(0).set(0, 1, 2);
    mockData.triangle(1).set(1, 3, 2);
    tMesh = new TriangleMesh(std::move(mockData));
  }

  // 2. Construção da Malha Half-Edge (Objetivo A1)
  HalfEdgeMesh heMesh(*tMesh);

  // 3. Exibição Estatística de Validação Topológica
  std::cout << "\n--- Estatisticas da Malha Half-Edge Concluida ---\n";
  std::cout << "Vertices Totais:  " << heMesh.vertices.size() << '\n';
  std::cout << "Semi-Arestas:     " << heMesh.halfEdges.size() << '\n';
  std::cout << "Arestas Unicas:   " << heMesh.edges.size() << '\n';
  std::cout << "Faces Totais:     " << heMesh.faces.size() << '\n';
  std::cout << "Contornos (Bordas): " << heMesh.boundaries.size() << '\n';

  // 4. Teste de Iteradores Básicos (Objetivo A2)
  size_t countVertices = 0;
  for (auto it = heMesh.vertexIter(); it != heMesh.vertices.end(); ++it) {
    countVertices++;
  }
  std::cout << "[Iterador] Varredura de vertices confirmada: " << countVertices << " processados.\n";

  // 5. Teste de Consultas de Vizinhança: Arestas Incidentes (Objetivo A2)
  index_t targetVertex = 1; // Vértice de teste
  std::cout << "\n[Vizinhanca] Arestas incidentes ao Vertice ID " << targetVertex << ":\n";
  heMesh.foreachIncidentEdge(targetVertex, [](index_t edgeId) {
    std::cout << " -> Ligado a Aresta Fisica ID: " << edgeId << '\n';
  });

  // 6. Teste de Consultas Avançadas: k-Anéis de Faces (Objetivo A2)
  size_t kDepth = 1;
  std::cout << "\n[Vizinhanca] Faces pertencentes ao " << kDepth << "-Anel do Vertice " << targetVertex << ":\n";
  heMesh.foreachVertexKRingFaces(targetVertex, kDepth, [](index_t faceId) {
    std::cout << " -> Inclui a Face ID: " << faceId << '\n';
  });

  // 7. Execução do Pipeline de Decoração de 3 Estágios (Objetivo A3 e A4)
  std::cout << "\n--- Execucao do Pipeline de Decoracoes ---\n";
  auto md1 = stage1_init(heMesh);
  auto md2 = stage2_metrics(md1.get());
  auto md3 = stage3_finalize(md2.get());

  // 8. Verificação final de extração segura de dados do SoA
  std::cout << "\n--- Extracao de Atributos via Decoracao Decoupled ---\n";
  std::cout << "ID verificado no Vertice 0: " << get<VD, 0>(*md3, 0) << "\n";
  std::cout << "Componente R da Face 0:      " << get<FD, 0>(*md3, 0).r << "\n";

  std::cout << "\n==================================================\n";
  std::cout << "Press ENTER to exit...\n";
  (void)getchar();
  return 0;
}
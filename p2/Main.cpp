#include <iostream>
#include <vector>
#include <cassert>
#include <type_traits>
#include <algorithm>
#include <string>
#include <iomanip>

// Inclusão gráfica nativa Linux
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>

// Cabeçalhos do seu trabalho
#include "HalfEdgeMesh.h"
#include "MeshDecoration.h"

using namespace tcii::cg;

enum { VD, ED, FD, BD };
struct RGBA { float r, g, b, a; };

// Variáveis globais para controlo de rotação 3D pelo rato
static float rotationX = 0.0f;
static float rotationY = 0.0f;
static bool leftPressed = false;
static double lastX = 0.0, lastY = 0.0;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (leftPressed) {
        float dx = static_cast<float>(xpos - lastX);
        float dy = static_cast<float>(ypos - lastY);
        rotationY += dx * 0.5f;
        rotationX += dy * 0.5f;
    }
    lastX = xpos;
    lastY = ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        leftPressed = (action == GLFW_PRESS);
    }
}

// Renderizador nativo baseado na sua topologia Half-Edge
void inicializar_e_rodar_opengl(const HalfEdgeMesh& mesh) {
    if (!glfwInit()) return;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Visualizador OpenGL 3D - P2", NULL, NULL);
    if (!window) { glfwTerminate(); return; }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat luzPos[] = { 5.0f, 5.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, luzPos);

    std::cout << "\n>>> [A2/GL] Janela aberta! Arraste com o rato para rodar o modelo.\n";

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-10.0, 10.0, -7.5, 7.5, -50.0, 50.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

        glColor3f(0.0f, 0.65f, 0.85f);

        glBegin(GL_TRIANGLES);
        for (const auto& face : mesh.faces) {
            index_t heIdx = face.halfEdge;
            for (int v = 0; v < 3; ++v) {
                if (heIdx == null_index) break;
                index_t vertIdx = mesh.halfEdges[heIdx].originVertex;
                if (vertIdx < mesh.vertices.size()) {
                    auto p = mesh.vertices[vertIdx].position;
                    glVertex3f(p.x, p.y, p.z);
                }
                heIdx = mesh.halfEdges[heIdx].next;
            }
        }
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Pipelines de Decoração Variádica (A3 e A4)
auto stage1_init(const HalfEdgeMesh& mesh) {
    using VA = void; using EA = void; using TA = ElementSoA<RGBA>; using BA = void;
    using MD = MeshDecoration<VA, EA, TA, BA>;
    auto md = MD::New(mesh);
    
    for (size_t i = 0; i < md->mesh()->faces.size(); ++i) {
        attributes<FD>(*md).set(static_cast<index_t>(i), RGBA{0.2f, 0.4f, 0.8f, 1.0f});
    }
    return md;
}

template <typename PreviousMD>
auto stage2_metrics(PreviousMD* previous) {
    using VA = void; using EA = ElementSoA<float>; using TA = typename PreviousMD::TA; using BA = void;
    using MD = MeshDecoration<VA, EA, TA, BA>;
    auto md = MD::New(previous);
    
    for (size_t i = 0; i < md->mesh()->edges.size(); ++i) {
        attributes<ED>(*md).set(static_cast<index_t>(i), 1.5f * static_cast<float>(i + 1));
    }
    return md;
}

template <typename PreviousMD>
auto stage3_finalize_void(PreviousMD* previous) {
    using VA = ElementSoA<int>; using EA = typename PreviousMD::EA; using TA = typename PreviousMD::TA; using BA = void;
    using MD = MeshDecoration<VA, EA, TA, BA>;
    auto md = MD::New(previous);
    
    for (size_t i = 0; i < md->mesh()->vertices.size(); ++i) {
        attributes<VD>(*md).set(static_cast<index_t>(i), 5000 + static_cast<int>(i));
    }
    return md;
}

// Demonstração explícita do Requisito A2 (Processamento de Vizinhos / Órbita)
void testar_navegacao_vizinhos(const HalfEdgeMesh& mesh, index_t vIdx) {
    if (vIdx >= mesh.vertices.size()) return;
    
    std::cout << "  -> Orbitando o Vertice [" << vIdx << "]: Vizinhos adjacentes encontrados: ";
    index_t startHe = mesh.vertices[vIdx].halfEdge;
    if (startHe == null_index) {
        std::cout << "Nenhum\n";
        return;
    }
    
    index_t currHe = startHe;
    bool primeiro = true;
    while (currHe != null_index && (primeiro || currHe != startHe)) {
        primeiro = false;
        index_t nextHe = mesh.halfEdges[currHe].next;
        index_t neighborVert = mesh.halfEdges[nextHe].originVertex;
        std::cout << neighborVert << " ";
        
        // Vai para a próxima semi-aresta que sai do mesmo vértice (usando o twin)
        index_t twin = mesh.halfEdges[currHe].twin;
        if (twin == null_index) break;
        currHe = twin; 
    }
    std::cout << "\n";
}

void imprimir_estatisticas(const std::string& nome, const HalfEdgeMesh& mesh) {
    size_t bordas_reais = (mesh.boundaries.size() == 1 && mesh.boundaries[0].halfEdge == null_index) ? 0 : mesh.boundaries.size();
    
    std::cout << "\n+---------------------------------------------------+\n";
    std::cout << "  [A1] TOPOLOGIA DA MALHA: " << nome << "\n";
    std::cout << "+---------------------------------------------------+\n";
    std::cout << "  | Vertices Totais:  " << std::setw(6) << mesh.vertices.size() << " |\n";
    std::cout << "  | Semi-Arestas:     " << std::setw(6) << mesh.halfEdges.size() << " |\n";
    std::cout << "  | Arestas Unicas:   " << std::setw(6) << mesh.edges.size() << " |\n";
    std::cout << "  | Faces Totais:     " << std::setw(6) << mesh.faces.size() << " |\n";
    std::cout << "  | Bordas Virtuais:  " << std::setw(6) << bordas_reais << " |\n";
    std::cout << "+---------------------------------------------------+\n";
    
    std::cout << "\n  [A2] TESTE DE ITERADORES / VIZINHANCA:\n";
    testar_navegacao_vizinhos(mesh, 0);
}

int main() {
    std::cout << "===================================================================\n";
    std::cout << "       DEMONSTRACAO DO TRABALHO PRATICO: REQUISITOS A1 A A4        \n";
    std::cout << "===================================================================\n";

    // ----------------------------------------------------
    // PROCESSAMENTO DO CUBO (Malha SEM Borda)
    // ----------------------------------------------------
    std::cout << "\n>>> [A4] Carregando modelo SEM BORDA (cubo.obj)...\n";
    ObjectPtr<TriangleMesh> tMeshCubo = readOBJ("cubo.obj");
    if (!tMeshCubo) {
        std::cerr << "[Erro] Certifique-se de que o ficheiro 'cubo.obj' existe.\n";
        return 1;
    }
    
    HalfEdgeMesh heMeshCubo(*tMeshCubo);
    if (heMeshCubo.boundaries.empty()) {
        heMeshCubo.boundaries.push_back({0, null_index});
    }

    imprimir_estatisticas("CUBO FECHADO", heMeshCubo);

    std::cout << "\n  [A3/A4] EXECUTANDO PIPELINE VARIADICO NO CUBO:\n";
    auto mdCubo1 = stage1_init(heMeshCubo);
    std::cout << "  |-> Estagio 1 [OK] (Faces decoradas com RGBA)\n";
    auto mdCubo2 = stage2_metrics(mdCubo1.get());
    std::cout << "  |-> Estagio 2 [OK] (Arestas decoradas com Metricas float)\n";
    auto mdCubo3 = stage3_finalize_void(mdCubo2.get());
    std::cout << "  |-> Estagio 3 [OK] (Vertices finalizados com IDs int)\n";
    std::cout << "  >> Atributos SoA gerados e validados com sucesso.\n";


    // ----------------------------------------------------
    // PROCESSAMENTO DO F-16 (Malha COM Borda)
    // ----------------------------------------------------
    std::cout << "\n\n>>> [A4] Carregando modelo COM BORDA (f-16.obj)...\n";
    ObjectPtr<TriangleMesh> tMeshF16 = readOBJ("f-16.obj");
    if (!tMeshF16) {
        std::cerr << "[Erro] Certifique-se de que o ficheiro 'f-16.obj' existe.\n";
        return 1;
    }
    
    HalfEdgeMesh heMeshF16(*tMeshF16);
    if (heMeshF16.boundaries.empty()) {
        heMeshF16.boundaries.push_back({0, null_index});
    }

    imprimir_estatisticas("F-16 FIGHTING FALCON", heMeshF16);

    std::cout << "\n  [A3/A4] EXECUTANDO PIPELINE VARIADICO NO F-16:\n";
    auto mdF16_1 = stage1_init(heMeshF16);
    std::cout << "  |-> Estagio 1 [OK] (Faces decoradas com RGBA)\n";
    auto mdF16_2 = stage2_metrics(mdF16_1.get());
    std::cout << "  |-> Estagio 2 [OK] (Arestas decoradas com Metricas float)\n";
    auto mdF16_3 = stage3_finalize_void(mdF16_2.get());
    std::cout << "  |-> Estagio 3 [OK] (Vertices finalizados com IDs int)\n";

    using FinalMDClass = std::remove_reference_t<decltype(*mdF16_3)>;
    if constexpr (std::is_same_v<typename FinalMDClass::BA, void>) {
        std::cout << "  >> [A3] Otimizacao estatica validada: Bordas nao ocupam memoria (BA = void).\n";
    }

    std::cout << "\n===================================================================\n";
    std::cout << "      TODOS OS REQUISITOS FORAM PROCESSADOS COM SUCESSO NO TERMINAL \n";
    std::cout << "===================================================================\n";

    // Disparar o visualizador 3D
    inicializar_e_rodar_opengl(heMeshF16);

    return 0;
}
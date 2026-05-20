#include "PointTraits.h"
#include "Array.h"
#include "RangeTree.h"
#include "Utils.h"
#include <cstdio>
#include <cstdlib>
#include <chrono>

// ============================================================================
// [1] DEFINIÇÃO DOS TIPOS DE PONTOS E ESPECIALIZAÇÕES DO POINTTRAITS
// ============================================================================

struct Ponto2D {
    using value_type = double;
    double coords[2];
    double operator[](size_t idx) const { return coords[idx]; }
    double& operator[](size_t idx) { return coords[idx]; }
};

struct Ponto3D {
    using value_type = double;
    double coords[3];
    double operator[](size_t idx) const { return coords[idx]; }
    double& operator[](size_t idx) { return coords[idx]; }
};

namespace tcii::cg {
    template <> struct PointTraits<Ponto2D> {
        static constexpr size_t dim = 2;
        using Bounds = double[2][dim];
    };
    template <> struct PointTraits<Ponto3D> {
        static constexpr size_t dim = 3;
        using Bounds = double[2][dim];
    };
}

using PointArray2D = tcii::cg::Array<Ponto2D>;
using PointArray3D = tcii::cg::Array<Ponto3D>;

// ============================================================================
// [2] PROCESSO PRINCIPAL COM ISOLAMENTO DE ESCOPO
// ============================================================================
int main(int argc, char** argv) {
    std::printf("==================================================================\n");
    std::printf("     BENCHMARK CORRIGIDO (SEM ALTERAR UTILS.H)                     \n");
    std::printf("==================================================================\n");

    constexpr size_t TOTAL_PONTOS = 9999999;
    auto callback_dummy = [](const auto&, size_t) -> bool { return true; };

    // ------------------------------------------------------------------------
    // FASE 1: TESTE EM 2D (Isolado em um bloco de escopo próprio)
    // ------------------------------------------------------------------------
    {
        std::printf("\n--- BENCHMARK 2D ($R^2$) com %zu pontos ---\n", TOTAL_PONTOS);

        // O uso de = {} zera toda a estrutura na memória prevenindo vazamentos
        typename tcii::cg::PointTraits<Ponto2D>::Bounds limites_espaco = {};
        limites_espaco[0][0] = -5000.0; limites_espaco[1][0] = 5000.0; // X
        limites_espaco[0][1] = -5000.0; limites_espaco[1][1] = 5000.0; // Y

        tcii::cg::PointSource<Ponto2D, PointArray2D> gerador;
        PointArray2D pontos = gerador.random(TOTAL_PONTOS, limites_espaco);

        auto inicio_build = std::chrono::high_resolution_clock::now();
        tcii::cg::RangeTree<Ponto2D, PointArray2D> arvore(pontos);
        arvore.build();
        auto fim_build = std::chrono::high_resolution_clock::now();
        std::printf("  [Construção]: %.2f ms\n", std::chrono::duration<double, std::milli>(fim_build - inicio_build).count());

        // Janela de busca restrita ao centro (captura ~4% do volume total do espaço)
        typename tcii::cg::PointTraits<Ponto2D>::Bounds caixa_busca = {};
        caixa_busca[0][0] = -1000.0; caixa_busca[1][0] = 1000.0;
        caixa_busca[0][1] = -1000.0; caixa_busca[1][1] = 1000.0;

        auto inicio_query = std::chrono::high_resolution_clock::now();
        size_t encontrados = arvore.query(caixa_busca, callback_dummy);
        auto fim_query = std::chrono::high_resolution_clock::now();

        std::printf("  [Consulta]:    %.4f ms (Pontos encontrados: %zu / %zu)\n", 
                    std::chrono::duration<double, std::milli>(fim_query - inicio_query).count(), encontrados, TOTAL_PONTOS);
    }

    // ------------------------------------------------------------------------
    // FASE 2: TESTE EM 3D (Isolado em seu próprio bloco de escopo)
    // ------------------------------------------------------------------------
    {
        std::printf("\n--- BENCHMARK 3D ($R^3$) com %zu pontos ---\n", TOTAL_PONTOS);

        typename tcii::cg::PointTraits<Ponto3D>::Bounds limites_espaco = {};
        limites_espaco[0][0] = -1000.0; limites_espaco[1][0] = 1000.0; // X
        limites_espaco[0][1] = -1000.0; limites_espaco[1][1] = 1000.0; // Y
        limites_espaco[0][2] = -1000.0; limites_espaco[1][2] = 1000.0; // Z

        tcii::cg::PointSource<Ponto3D, PointArray3D> gerador;
        PointArray3D pontos = gerador.random(TOTAL_PONTOS, limites_espaco);

        auto inicio_build = std::chrono::high_resolution_clock::now();
        tcii::cg::RangeTree<Ponto3D, PointArray3D> arvore(pontos);
        arvore.build();
        auto fim_build = std::chrono::high_resolution_clock::now();
        std::printf("  [Construção]: %.2f ms\n", std::chrono::duration<double, std::milli>(fim_build - inicio_build).count());

        // Janela de busca restrita (captura ~1.5% do volume total do hipervolume)
        typename tcii::cg::PointTraits<Ponto3D>::Bounds caixa_busca = {};
        caixa_busca[0][0] = -250.0; caixa_busca[1][0] = 250.0;
        caixa_busca[0][1] = -250.0; caixa_busca[1][1] = 250.0;
        caixa_busca[0][2] = -250.0; caixa_busca[1][2] = 250.0;

        auto inicio_query = std::chrono::high_resolution_clock::now();
        size_t encontrados = arvore.query(caixa_busca, callback_dummy);
        auto fim_query = std::chrono::high_resolution_clock::now();

        std::printf("  [Consulta]:    %.4f ms (Pontos encontrados: %zu / %zu)\n", 
                    std::chrono::duration<double, std::milli>(fim_query - inicio_query).count(), encontrados, TOTAL_PONTOS);
    }

    std::printf("\n==================================================================\n");
    std::printf("                     FIM DOS TESTES DE VOLUMETRIA                   \n");
    std::printf("==================================================================\n");
    return EXIT_SUCCESS;
}
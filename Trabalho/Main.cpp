#include "PointTraits.h"
#include "Array.h"
#include "RangeTree.h"
#include "Utils.h"
#include <cstdio>
#include <cstdlib>
#include <chrono>

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

int main(int argc, char** argv) {
    std::printf("==================================================================\n");
    std::printf("                       INICIO DOS TESTES                     \n");
    std::printf("==================================================================\n");

    constexpr size_t TOTAL_PONTOS = 200;
    auto callback_dummy = [](const auto&, size_t) -> bool { return true; };

    // Dimensao 2d
    {
        std::printf("\n--- DIMENSAO 2D COM %zu PONTOS ---\n", TOTAL_PONTOS);

        // Previnir vazamentos
        typename tcii::cg::PointTraits<Ponto2D>::Bounds limites_espaco = {};
        limites_espaco[0][0] = -5000.0; limites_espaco[1][0] = 5000.0; // X
        limites_espaco[0][1] = -5000.0; limites_espaco[1][1] = 5000.0; // Y

        tcii::cg::PointSource<Ponto2D, PointArray2D> gerador;
        PointArray2D pontos = gerador.random(TOTAL_PONTOS, limites_espaco);

        auto inicio_build = std::chrono::high_resolution_clock::now();
        tcii::cg::RangeTree<Ponto2D, PointArray2D> arvore(pontos);
        arvore.build();
        auto fim_build = std::chrono::high_resolution_clock::now();
        std::printf("\n  [Construção]: %.2f ms\n", std::chrono::duration<double, std::milli>(fim_build - inicio_build).count());

        // Busca
        typename tcii::cg::PointTraits<Ponto2D>::Bounds caixa_busca = {};
        caixa_busca[0][0] = -1000.0; caixa_busca[1][0] = 1000.0;
        caixa_busca[0][1] = -1000.0; caixa_busca[1][1] = 1000.0;

        auto inicio_query = std::chrono::high_resolution_clock::now();
        size_t encontrados = arvore.query(caixa_busca, callback_dummy);
        auto fim_query = std::chrono::high_resolution_clock::now();

        std::printf("  [Consulta]: %.4f ms \n  [Pontos encontrados]: %zu / %zu\n", 
                    std::chrono::duration<double, std::milli>(fim_query - inicio_query).count(), encontrados, TOTAL_PONTOS);
        
        std::printf("\n=======================FIM DA DIMENSAO 2D=========================");
    }

    // Dimensao 3d
    {
        std::printf("\n\n--- DIMENSAO 3D COM %zu PONTOS ---\n", TOTAL_PONTOS);

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
        std::printf("\n  [Construção]: %.2f ms\n", std::chrono::duration<double, std::milli>(fim_build - inicio_build).count());

        // Busca
        typename tcii::cg::PointTraits<Ponto3D>::Bounds caixa_busca = {};
        caixa_busca[0][0] = -250.0; caixa_busca[1][0] = 250.0;
        caixa_busca[0][1] = -250.0; caixa_busca[1][1] = 250.0;
        caixa_busca[0][2] = -250.0; caixa_busca[1][2] = 250.0;

        auto inicio_query = std::chrono::high_resolution_clock::now();
        size_t encontrados = arvore.query(caixa_busca, callback_dummy);
        auto fim_query = std::chrono::high_resolution_clock::now();

        std::printf("  [Consulta]: %.4f ms  \n  [Pontos encontrados]: %zu / %zu\n", 
                    std::chrono::duration<double, std::milli>(fim_query - inicio_query).count(), encontrados, TOTAL_PONTOS);
    }
    std::printf("\n=======================FIM DA DIMENSAO 3D=========================");

    std::printf("\n==================================================================\n");
    std::printf("                       FIM DOS TESTES                  \n");
    std::printf("==================================================================\n");
    return EXIT_SUCCESS;
}

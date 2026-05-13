#ifndef __RangeTree_h
#define __RangeTree_h

#include "utils.h"
#include "PointTraits.h"
#include <functional>
#include <numeric>

namespace tcii::cg{ // Begin namespace tcii::cg
    namespace rtree{ // Begin namespace rtree
        using index_t = unsigned;
        using IndexArray = Array<index_t>;

        template <size_t D, typename P> // Formatação genérica do template
        inline auto _x(const P& p){ // declaração de função para casos de polimorfismo (_x == classe/tipos _x para diferenciar de variáveis  x)
            if constexpr (std::is_arithmetic_v<P>){ // sinaliza execuçao em tempo de copilação
                static_assert(D == 1);
                return p;
            }
            else
                return p[D - 1];
        }

        template <typename A>
        using PointFunc = std::function<bool(const A&, size_t)>;

        template <size_t D, typename P, typename A> // Versão genérica para casos D > 1
        class BBST;
        template <typename P, typename A> // Especialização para o caso base D == 1
        class BBST<1, P, A>{
            public:
            using Bounds = typename PointTraits<P>::Bounds;
            using PointFunc = rtree::PointFunc<A>;

            void build(const A& points){
                struct Node {
                    real coord; //Capta as coordenadas do nó
                    IndexArray canonical; // Índice dos pontos do nó 
                    Node* left{}; // Nó esquerdo
                    Node* right{}; // Nó direito
                    AssociatedTree* assoc{}; // Pega a arvore inferior dentro do nó
                }
    
            }
 
            size_t query(const A& points, const Bounds& bounds, PointFunc f) const{
                if (Node->coord > 1){
                    assoc->query(left);
                    assoc->query(right);
                }
                else if (Node->coord = 1){
                    query();
                }
                else{
                    query(null);
                }
                return 0;
            }

        }; // BBST 

        template <size_t D, typename P, typename A> // Versão genérica para casos D > 1
        class BBST{
            public:
            using Bounds = typename PointTraits<P>::Bounds;
            using PointFunc = rtree::PointFunc<A>;

            ~BBST(){
                delete _root;
            }

            void build(const A& points){ // Construir Árvore
                assert(!_root); // Assegura que a árvore estará vazia
                _indices.resize(points.size()); //Manipula tamanho de armazenamento de indices para criar espaço

                std::iota(_indices.begin(), _indices.end()), 0; //Gera indices dentro de um intervalo

                std::sort(_indices.begin(), _indices.end(), //Ordena o intervalo de indices 
                [&](auto a, auto b){ //Expressão Lambda
                    return _x<D>(points[a])
                        < _x<D>(points[b]);
                }
                );

                _root = buildRecursive(points, _indices); // Inicia a Recursão

            }

            size_t query(const A& points, const Bounds& bounds, PointFunc f) const{
                if (Node->coord >= min && Node->coord <= max){
                    query(left);
                    query(right);
                }
                else if (Node->coord < min){
                    query(right);
                }
                else{
                    query(left);
                }
                return 0;
            }
            private:
            using real = typename P::value_type;
            using AssociatedTree = BBST<D - 1, P, A>;

            struct Node {
                real pivot; // Capta as coordenadas/pontos de divisão do nó (a e b, esquerda e direita)
                IndexArray canonical; // Índice dos pontos do nó 
                Node* left{}; // Nó esquerdo
                Node* right{}; // Nó direito
                AssociatedTree* assoc{}; // Pega a arvore inferior dentro do nó

                ~Node(){ // Limpa alocação
                    delete left;
                    delete right;
                    delete assoc;
                }
            };
            Node* buildRecursive(
                const A& points,
                const IndexArray& indices){

                    if (indices.empty()){ //Se não tiver nada, nao retorna nada
                        return nullptr;
                    }

                    Node* node = new Node; // Aloca um novo nó
                    node->canonical = indices; // Guarda os índices dos pontos pertencentes ao subconjunto de nós
                    size_t mid = indices.size() / 2; // Calcula Mediana/Ponto Central
                    node->pivot = _x<D>(points[indices[mid]]); // Separa os indices
                    IndexArray leftindices; // Define quem é esquerda
                    IndexArray rightindices; // Define quem é Direita
                    // Node* _root{};

                    leftindices.insert(
                        leftindices.end(), indices.begin(), indices.begin() + mid
                    );

                    rightindices.insert(
                        rightindices.end(), indices.begin() + mid + 1, indices.end()
                    );

                    node->left = buildRecursive(points, leftindices); // Constrói sub-árvores esquerdas
                    node->right = buildRecursive(points, rightindices); // Constrói sub-árvores dreitas
                    node->assoc = new AssociatedTree; // Conecta as sub-árvores
                    node->assoc->build(points); // Monta as sub-árvores

                    return node;
                }
                Node* _root{};
        }; // BBST

    } // end namespace rtree

    template <typename P, typename A>
    class RangeTree{
        public:
        constexpr static auto D = point_dim_v<P>;

        using Bounds = typename PointTraits<P>::Bounds;
        using PointFunc = rtree::PointFunc<A>;

        RangeTree(const A& points):
        _points{points}{
            // do nothing
        }

        auto& points() const{
            return _points;
        }

        void build(){
            _mainTree.build(_points);
        }

        auto query(const Bounds& bounds, PointFunc f) const{
            return _mainTree.query(_points, bounds, f);
        }

        private:
        const A& _points;
        rtree::BBST<D, P, A> _mainTree;

    }; // RangeTree

} // end namespace tcii::cg
#endif // __RangeTree_h



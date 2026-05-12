#ifndef __RangeTree_h
#define __RangeTree_h

#include "util/Array.h"
#include "PointTraits.h"
#include <functional>
#include <numeric>

namespace tcii::cg{ // begin namespace tcii::cg
    namespace rtree{ // begin namespace rtree
        using index_t = unsigned;
        using IndexArray = Array<index_t>;

        template <size_t D, typename P> // Formatação genérica do template
        inline auto _x(const P& p){ // declaração de função para casos de polimorfismo
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
                // escreva seu código aqui
    
            }
 
            size_t query(const A& points, const Bounds& bounds, PointFunc f) const{
                // escreva seu código aqui
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

            void build(const A& points){
                assert(!_root);
                struct Node {
                    real coord; //capta as coordenadas do nó
                    indexArray canonical; // índice dos pontos do nó 
                    Node* left{};
                    Node* right{};
                    AssociatedTree* assoc{}; //pega a arvore inferior dentro do nó
                }
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
                    real pivot; //capta as coordenadas do nó
                    indexArray canonical; // índice dos pontos do nó 
                    Node* left{};
                    Node* right{};
                    AssociatedTree* assoc{}; //pega a arvore inferior dentro do nó
                } // Node

            Node* _root{};
            IndexArray _indices;

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


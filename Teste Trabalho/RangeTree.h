#ifndef __RangeTree_h
#define __RangeTree_h

#include "Array.h"
#include "Utils.h"
#include "PointTraits.h"
#include <functional>
#include <numeric>
#include <algorithm> // Necessário para std::sort
#include <cassert>   // Necessário para assert

namespace tcii::cg{ // Begin namespace tcii::cg
    namespace rtree{ // Begin namespace rtree
        using index_t = unsigned;
        using IndexArray = Array<index_t>;

        template <size_t D, typename P> 
        inline auto _x(const P& p){ 
            if constexpr (std::is_arithmetic_v<P>){ 
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

            // Adicionado destrutor para evitar vazamento de memória da estrutura original
            ~BBST() { delete _root; }

            void build(const A& points){
                // O caso base D=1 recebe todos os pontos na primeira chamada.
                _indices.resize(points.size());
                std::iota(_indices.begin(), _indices.end(), 0);

                std::sort(_indices.begin(), _indices.end(), 
                    [&](auto a, auto b){ 
                        return _x<1>(points[a]) < _x<1>(points[b]);
                    }
                );
                _root = buildRecursive(points, _indices);
            }
 
            size_t query(const A& points, const Bounds& bounds, PointFunc f) const{
                return queryRecursive(_root, points, bounds, f);
            }

            private:
            using real = typename P::value_type;

            struct Node {
                real pivot; // Corrigido de coord para pivot (consistência com D > 1)
                IndexArray canonical; 
                Node* left{}; 
                Node* right{}; 
                // Removido AssociatedTree* assoc no caso D=1, pois não existe dimensão abaixo de 1
                
                ~Node() {
                    delete left;
                    delete right;
                }
            }; // Corrigido: adicionado ponto e vírgula na struct

            Node* buildRecursive(const A& points, const IndexArray& indices) {
                if (indices.empty()) return nullptr;

                Node* node = new Node;
                node->canonical = indices;
                size_t mid = indices.size() / 2;
                node->pivot = _x<1>(points[indices[mid]]);

                IndexArray leftindices(indices.begin(), indices.begin() + mid);
                IndexArray rightindices(indices.begin() + mid + 1, indices.end());

                node->left = buildRecursive(points, leftindices);
                node->right = buildRecursive(points, rightindices);
                return node;
            }

            size_t queryRecursive(Node* node, const A& points, const Bounds& bounds, PointFunc f) const {
                if (!node) return 0;

                real min_b = bounds.min[0];
                real max_b = bounds.max[0];
                size_t count = 0;

                if (node->pivot >= min_b && node->pivot <= max_b) {
                    for (auto idx : node->canonical) {
                        if (f(points, idx)) count++;
                    }
                    return count;
                }
                if (node->pivot > min_b) count += queryRecursive(node->left, points, bounds, f);
                if (node->pivot < max_b) count += queryRecursive(node->right, points, bounds, f);
                return count;
            }

            Node* _root{};
            IndexArray _indices;
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
                _indices.resize(points.size()); 

                std::iota(_indices.begin(), _indices.end(), 0); // Corrigido: fechamento do parêntese do std::iota

                std::sort(_indices.begin(), _indices.end(), 
                [&](auto a, auto b){ 
                    return _x<D>(points[a]) < _x<D>(points[b]);
                }
                );

                _root = buildRecursive(points, _indices); 
            }

            // Interface pública mantida idêntica à original
            size_t query(const A& points, const Bounds& bounds, PointFunc f) const{
                return queryRecursive(_root, points, bounds, f);
            }

            private:
            using real = typename P::value_type;
            using AssociatedTree = BBST<D - 1, P, A>;

            struct Node {
                real pivot; 
                IndexArray canonical; 
                Node* left{}; 
                Node* right{}; 
                AssociatedTree* assoc{}; 

                ~Node(){ 
                    delete left;
                    delete right;
                    delete assoc;
                }
            };

            // Lógica de busca corrigida respeitando o escopo do nó
            size_t queryRecursive(Node* node, const A& points, const Bounds& bounds, PointFunc f) const {
                if (!node) return 0;

                real min_b = bounds.min[D - 1];
                real max_b = bounds.max[D - 1];
                size_t count = 0;

                if (node->pivot >= min_b && node->pivot <= max_b) {
                    if (node->assoc) {
                        count += node->assoc->query(points, bounds, f);
                    }
                    return count;
                }
                if (node->pivot > min_b) count += queryRecursive(node->left, points, bounds, f);
                if (node->pivot < max_b) count += queryRecursive(node->right, points, bounds, f);
                return count;
            }

            Node* buildRecursive(
                const A& points,
                const IndexArray& indices){

                    if (indices.empty()){ 
                        return nullptr;
                    }

                    Node* node = new Node; 
                    node->canonical = indices; 
                    size_t mid = indices.size() / 2; 
                    node->pivot = _x<D>(points[indices[mid]]); 
                    
                    // Corrigido: Sintaxe de inicialização dos sub-vetores para evitar que fiquem vazios
                    IndexArray leftindices(indices.begin(), indices.begin() + mid); 
                    IndexArray rightindices(indices.begin() + mid + 1, indices.end()); 

                    node->left = buildRecursive(points, leftindices); 
                    node->right = buildRecursive(points, rightindices); 
                    
                    node->assoc = new AssociatedTree; 
                    
                    // CORREÇÃO LOGICA DA RT: A subárvore não reconstrói a lista inteira, 
                    // ela constrói apenas com base nos pontos contidos na subárvore deste nó atual.
                    // Para se ajustar à assinatura original build(const A&), criamos uma estrutura auxiliar local.
                    A canonical_points;
                    for(auto idx : node->canonical) {
                        canonical_points.push_back(points[idx]);
                    }
                    node->assoc->build(canonical_points); 

                    return node;
                }
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
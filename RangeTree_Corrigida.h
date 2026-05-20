#ifndef __RangeTree_h
#define __RangeTree_h

#include "Utils.h"
#include "PointTraits.h"
#include <functional>
#include <numeric>
#include <algorithm>
#include <cassert>

namespace tcii::cg { 
    namespace rtree { 
        using index_t = unsigned;
        using IndexArray = Array<index_t>;

        template <size_t D, typename P> 
        inline auto _x(const P& p) { 
            if constexpr (std::is_arithmetic_v<P>) { 
                static_assert(D == 1);
                return p;
            } else {
                return p[D - 1];
            }
        }

        template <typename A>
        using PointFunc = std::function<bool(const A&, size_t)>;

    
        // D = 1
        template <typename P, typename A> 
        class BBST<1, P, A> {
        public:
            using Bounds = typename PointTraits<P>::Bounds;
            using PointFunc = rtree::PointFunc<A>;

            ~BBST() { delete _root; }

            // Controi a arvore RT
            void build(const A& points, const IndexArray& indices) {
                _root = buildRecursive(points, indices);
            }

            size_t query(const A& points, const Bounds& bounds, PointFunc f) const {
                return queryRecursive(_root, points, bounds, f);
            }

        private:
            using real = decltype(_x<1>(std::declval<typename A::value_type>()));

            struct Node {
                real pivot; 
                IndexArray canonical; 
                Node* left{}; 
                Node* right{}; 

                ~Node() {
                    delete left;
                    delete right;
                }
            };

            Node* buildRecursive(const A& points, const IndexArray& indices) {
                if (indices.empty()) return nullptr;

                // Ordena os índices pela dimensão 1
                IndexArray sorted_indices = indices;
                std::sort(sorted_indices.begin(), sorted_indices.end(), [&](auto a, auto b) {
                    return _x<1>(points[a]) < _x<1>(points[b]);
                });

                Node* node = new Node;
                node->canonical = sorted_indices;
                
                size_t mid = sorted_indices.size() / 2;
                node->pivot = _x<1>(points[sorted_indices[mid]]);

                IndexArray leftIndices(sorted_indices.begin(), sorted_indices.begin() + mid);
                IndexArray rightIndices(sorted_indices.begin() + mid + 1, sorted_indices.end());

                node->left = buildRecursive(points, leftIndices);
                node->right = buildRecursive(points, rightIndices);

                return node;
            }

            size_t queryRecursive(Node* node, const A& points, const Bounds& bounds, PointFunc f) const {
                if (!node) return 0;

                real min_bound = bounds.min[0]; // Dimensão 1 mapeia para índice 0
                real max_bound = bounds.max[0];
                size_t count = 0;

                // Se o nó atual estiver totalmente contido no intervalo 1D
                if (node->pivot >= min_bound && node->pivot <= max_bound) {
                    // Como é D=1, chegamos na última folha/dimensão. Executa a função f para os válidos
                    for (auto idx : node->canonical) {
                        // Verifica se passa em todas as restrições de bounds adicionais (caso venha de D superior)
                        if (f(points, idx)) count++;
                    }
                    return count;
                }

                if (node->pivot > min_bound) {
                    count += queryRecursive(node->left, points, bounds, f);
                }
                if (node->pivot < max_bound) {
                    count += queryRecursive(node->right, points, bounds, f);
                }
                return count;
            }

            Node* _root{};
        }; 

        // D > 1
        template <size_t D, typename P, typename A> 
        class BBST {
        public:
            using Bounds = typename PointTraits<P>::Bounds;
            using PointFunc = rtree::PointFunc<A>;

            ~BBST() { delete _root; }

            void build(const A& points, const IndexArray& indices) {
                _root = buildRecursive(points, indices);
            }

        private:
            using real = decltype(_x<D>(std::declval<typename A::value_type>()));
            using AssociatedTree = BBST<D - 1, P, A>;

            struct Node {
                real pivot; 
                IndexArray canonical; 
                Node* left{}; 
                Node* right{}; 
                AssociatedTree* assoc{}; 

                ~Node() { 
                    delete left;
                    delete right;
                    delete assoc;
                }
            };

            Node* buildRecursive(const A& points, const IndexArray& indices) {
                if (indices.empty()) return nullptr;

                // Ordena os pela dimensão D atual.
                IndexArray sorted_indices = indices;
                std::sort(sorted_indices.begin(), sorted_indices.end(), [&](auto a, auto b) {
                    return _x<D>(points[a]) < _x<D>(points[b]);
                });

                Node* node = new Node;
                node->canonical = sorted_indices;
                
                size_t mid = sorted_indices.size() / 2;
                node->pivot = _x<D>(points[sorted_indices[mid]]);

                IndexArray leftIndices(sorted_indices.begin(), sorted_indices.begin() + mid);
                IndexArray rightIndices(sorted_indices.begin() + mid + 1, sorted_indices.end());

                node->left = buildRecursive(points, leftIndices);
                node->right = buildRecursive(points, rightIndices);

                // Constrói a árvore associada com os índices do nó canônico.
                node->assoc = new AssociatedTree();
                node->assoc->build(points, node->canonical); 

                return node;
            }

        public:
            size_t query(const A& points, const Bounds& bounds, PointFunc f) const {
                return queryRecursive(_root, points, bounds, f);
            }

        private:
            size_t queryRecursive(Node* node, const A& points, const Bounds& bounds, PointFunc f) const {
                if (!node) return 0;

                real min_bound = bounds.min[D - 1];
                real max_bound = bounds.max[D - 1];
                size_t count = 0;

                // Se o nó atual cai dentro do intervalo da dimensão D, delega para a árvore associada (D-1)
                if (node->pivot >= min_bound && node->pivot <= max_bound) {
                    if (node->assoc) {
                        count += node->assoc->query(points, bounds, f);
                    }
                    return count;
                }

                if (node->pivot > min_bound) {
                    count += queryRecursive(node->left, points, bounds, f);
                }
                if (node->pivot < max_bound) {
                    count += queryRecursive(node->right, points, bounds, f);
                }
                return count;
            }

            Node* _root{};
        }; 

    } // end namespace rtree

    // Range Tree
    template <typename P, typename A>
    class RangeTree {
    public:
        constexpr static auto D = point_dim_v<P>;

        using Bounds = typename PointTraits<P>::Bounds;
        using PointFunc = rtree::PointFunc<A>;

        RangeTree(const A& points) : _points{points} {}

        auto& points() const { return _points; }

        void build() {
            rtree::IndexArray inicial_indices(_points.size());
            std::iota(inicial_indices.begin(), inicial_indices.end(), 0);
            _mainTree.build(_points, inicial_indices);
        }

        auto query(const Bounds& bounds, PointFunc f) const {
            return _mainTree.query(_points, bounds, f);
        }

    private:
        const A& _points;
        rtree::BBST<D, P, A> _mainTree;
    }; 

} // end namespace tcii::cg
#endif // __RangeTree_h

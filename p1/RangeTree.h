#ifndef __RangeTree_h
#define __RangeTree_h

// OVERVIEW: RangeTree.h
// ========
// Class definition for generic array.
//
// Author: Kimberlly Stachelski e Aron Petta Sarabia
// Last Revision: 20/05/2026

#include "Array.h"
#include "Utils.h"
#include "PointTraits.h"
#include <functional>
#include <numeric>
#include <algorithm>
#include <cassert>
#include <vector>

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

        template <size_t D, typename P, typename A>
        class BBST;

        // --- ESPECIALIZAÇÃO CASO BASE D = 1 ---
        template <typename P, typename A>
        class BBST<1, P, A> {
        public:
            using Bounds = typename PointTraits<P>::Bounds;
            using PointFunc = rtree::PointFunc<A>;

            ~BBST() { delete _root; }

            void build(const A& points, const std::vector<index_t>& active_indices) {
                std::vector<index_t> tmp = active_indices;
                std::sort(tmp.begin(), tmp.end(), [&](auto a, auto b) { 
                    return _x<1>(points[a]) < _x<1>(points[b]);
                });
                _indices = IndexArray(tmp.begin(), tmp.end());
                _root = buildRecursive(points, _indices);
            }

            size_t query(const A& points, const Bounds& bounds, PointFunc f) const {
                return queryRecursive(_root, points, bounds, f);
            }

        private:
            using real = typename P::value_type;
            struct Node {
                real pivot;
                index_t point_idx;
                Node* left{};
                Node* right{};
                ~Node() { delete left; delete right; }
            };

            Node* buildRecursive(const A& points, const IndexArray& indices) {
                if (indices.size() == 0) return nullptr;
                
                Node* node = new Node;
                size_t mid = indices.size() / 2;
                node->point_idx = indices[mid];
                node->pivot = _x<1>(points[indices[mid]]);

                IndexArray leftindices(indices.begin(), indices.begin() + mid);
                IndexArray rightindices(indices.begin() + mid + 1, indices.end());

                node->left = buildRecursive(points, leftindices);
                node->right = buildRecursive(points, rightindices);
                return node;
            }

            size_t queryRecursive(Node* node, const A& points, const Bounds& bounds, PointFunc f) const {
                if (!node) return 0;

                real min_b = bounds[0][0];
                real max_b = bounds[1][0];
                size_t count = 0;

                // Verifica se o ponto do nó atual está no intervalo 1D
                if (node->pivot >= min_b && node->pivot <= max_b) {
                    if (f(points, node->point_idx)) count++;
                }
                
                // Continua a busca na árvore binária em ambas as direções de forma correta
                if (node->pivot > min_b) count += queryRecursive(node->left, points, bounds, f);
                if (node->pivot < max_b) count += queryRecursive(node->right, points, bounds, f);
                return count;
            }

            Node* _root{};
            IndexArray _indices;
        };

        // --- CLASSE GENÉRICA PARA D > 1 ---
        template <size_t D, typename P, typename A>
        class BBST {
        public:
            using Bounds = typename PointTraits<P>::Bounds;
            using PointFunc = rtree::PointFunc<A>;

            ~BBST() { delete _root; }

            void build(const A& points, const std::vector<index_t>& active_indices) {
                std::vector<index_t> tmp = active_indices;
                std::sort(tmp.begin(), tmp.end(), [&](auto a, auto b) { 
                    return _x<D>(points[a]) < _x<D>(points[b]);
                });
                _indices = IndexArray(tmp.begin(), tmp.end());
                _root = buildRecursive(points, _indices);
            }

            size_t query(const A& points, const Bounds& bounds, PointFunc f) const {
                return queryRecursive(_root, points, bounds, f);
            }

        private:
            using real = typename P::value_type;
            using AssociatedTree = BBST<D - 1, P, A>;

            struct Node {
                real pivot;
                index_t point_idx;
                Node* left{};
                Node* right{};
                AssociatedTree* assoc{};
                ~Node() { delete left; delete right; delete assoc; }
            };

            Node* buildRecursive(const A& points, const IndexArray& indices) {
                if (indices.size() == 0) return nullptr;

                Node* node = new Node;
                size_t mid = indices.size() / 2;
                node->point_idx = indices[mid];
                node->pivot = _x<D>(points[indices[mid]]);

                IndexArray leftindices(indices.begin(), indices.begin() + mid);
                IndexArray rightindices(indices.begin() + mid + 1, indices.end());

                node->left = buildRecursive(points, leftindices);
                node->right = buildRecursive(points, rightindices);

                // IMPORTANTE: A árvore associada recebe TODOS os pontos do nó corrente (subárvore)
                node->assoc = new AssociatedTree;
                std::vector<index_t> current_indices;
                for (size_t i = 0; i < indices.size(); ++i) {
                    current_indices.push_back(indices[i]);
                }
                // Passa o vetor de pontos global inalterado, mantendo a integridade dos índices!
                node->assoc->build(points, current_indices);

                return node;
            }

            size_t queryRecursive(Node* node, const A& points, const Bounds& bounds, PointFunc f) const {
                if (!node) return 0;

                real min_b = bounds[0][D - 1];
                real max_b = bounds[1][D - 1];
                size_t count = 0;

                // Se o pivô atual corta o intervalo, desce na árvore associada da próxima dimensão
                if (node->pivot >= min_b && node->pivot <= max_b) {
                    if (node->assoc) {
                        count += node->assoc->query(points, bounds, f);
                    }
                } else {
                    // Se estiver fora, precisamos continuar pesquisando os ramos descendentes aplicáveis
                    if (node->pivot > min_b) count += queryRecursive(node->left, points, bounds, f);
                    if (node->pivot < max_b) count += queryRecursive(node->right, points, bounds, f);
                }
                return count;
            }

            Node* _root{};
            IndexArray _indices;
        };

    } // namespace rtree

    template <typename P, typename A>
    class RangeTree {
    public:
        constexpr static auto D = point_dim_v<P>;
        using Bounds = typename PointTraits<P>::Bounds;
        using PointFunc = rtree::PointFunc<A>;

        RangeTree(const A& points) : _points{points} {}

        auto& points() const { return _points; }

        void build() {
            std::vector<rtree::index_t> initial_indices(_points.size());
            std::iota(initial_indices.begin(), initial_indices.end(), 0);
            _mainTree.build(_points, initial_indices);
        }

        auto query(const Bounds& bounds, PointFunc f) const {
            return _mainTree.query(_points, bounds, f);
        }

    private:
        const A& _points;
        rtree::BBST<D, P, A> _mainTree;
    };

} // namespace tcii::cg
#endif // __RangeTree_h
#ifndef __MeshDecoration_h
#define __MeshDecoration_h

// OVERVIEW: MeshDecoration.h
// ========
// Class definition for triangle mesh decoration.
//
// Author: Paulo Pagliosa
// Last revision: 03/06/2026

#include "DecorationSet.h"
#include "HalfEdgeMesh.h" // Inclui sua nova malha Half-Edge

namespace tcii::cg
{ // begin namespace tcii::cg

/////////////////////////////////////////////////////////////////////
//
// MeshDecoration: classe expandida para decorações de Half-Edge Mesh
// ==============
template <typename V, typename E, typename T, typename B>
class EMPTY_BASES MeshDecoration: public SharedObject,
  public DecorationSet<V, E, T, B>
{
public:
  using VA = V;
  using EA = E;
  using TA = T;
  using BA = B;
  using Base = DecorationSet<V, E, T, B>;
  using pointer = ObjectPtr<MeshDecoration>;

  // Fábrica para criar a partir da malha Half-Edge construída
  static auto New(const HalfEdgeMesh& mesh)
  {
    return pointer{new MeshDecoration{&mesh}};
  }

  // Construtor de conversão movível para encadeamento de estágios (Pipeline)
  template <typename OtherV, typename OtherE, typename OtherT, typename OtherB>
  static auto New(MeshDecoration<OtherV, OtherE, OtherT, OtherB>* other)
  {
    assert(other != nullptr);
    return pointer{new MeshDecoration{std::move(*other)}};
  }

  const HalfEdgeMesh* mesh() const { return _mesh; }
  HalfEdgeMesh* mesh() { return const_cast<HalfEdgeMesh*>(_mesh); }

private:
  const HalfEdgeMesh* _mesh;

  // Inicializa as 4 tabelas de atributos com seus respectivos tamanhos
  MeshDecoration(const HalfEdgeMesh* mesh):
    Base{
      static_cast<index_t>(mesh->vertices.size()),
      static_cast<index_t>(mesh->edges.size()),
      static_cast<index_t>(mesh->faces.size()),
      static_cast<index_t>(mesh->boundaries.size())
    },
    _mesh{mesh}
  {
    // do nothing
  }

  MeshDecoration(MeshDecoration&&) noexcept = default;

  // Construtor move-constructor variádico essencial para o pipeline de dados
  template <typename OtherV, typename OtherE, typename OtherT, typename OtherB>
  MeshDecoration(MeshDecoration<OtherV, OtherE, OtherT, OtherB>&& other):
    Base{std::move(other)},
    _mesh{other.mesh()}
  {
    // do nothing
  }
};

} // end namespace tcii::cg

#endif // __MeshDecoration_h
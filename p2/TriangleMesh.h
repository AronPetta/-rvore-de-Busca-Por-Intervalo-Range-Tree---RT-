#ifndef __TriangleMesh_h
#define __TriangleMesh_h

// OVERVIEW: TriangleMesh.h
// ========
// Class definition for triangle mesh.
//
// Author: Paulo Pagliosa
// Last revision: 06/07/2025

#include "Bounds3.h"
#include "SharedObject.h"
#include "ArrayView.h"
#include <cstdio>

namespace tcii::cg
{ // begin namespace tcii::cg

template <typename T>
struct Index3
{
  union
  {
    struct { T x; T y; T z; };
    struct { T i; T j; T k; };
  };

  void set(T i, T j, T k)
  {
    x = i;
    y = j;
    z = k;
  }

  auto& operator [](T i)
  {
    assert(i < 3);
    return (&x)[i];
  }

  auto operator [](T i) const
  {
    return const_cast<Index3&>(*this)[i];
  }

}; // Index3


/////////////////////////////////////////////////////////////////////
//
// TriangleMesh: class for triangle mesh.
// ============
class TriangleMesh: public SharedObject
{
public:
  using index_t = unsigned;
  using vec3 = Vec3f;
  using Bounds = Bounds3f;
  using Triangle = Index3<index_t>;
  using TriangleArray = ArrayView<Triangle>;
  using Vec3Array = ArrayView<vec3>;

  class Data
  {
  public:
    Data(index_t vertexSize, index_t triangleSize);

    ~Data()
    {
      delete[]_vertices;
      delete[]_vertexNormals;
      delete[]_triangles;
    }

    auto vertexCount() const
    {
      return _vertexSize;
    }

    auto& vertex(index_t i)
    {
      assert(i < _vertexSize);
      return _vertices[i];
    }

    auto& vertex(index_t i) const
    {
      return const_cast<Data*>(this)->vertex(i);
    }

    Vec3Array vertices() const
    {
      return {_vertices, _vertexSize};
    }

    auto& vertexNormal(index_t i)
    {
      assert(_vertexNormals && i < _vertexSize);
      return _vertexNormals[i];
    }

    auto& vertexNormal(index_t i) const
    {
      return const_cast<Data*>(this)->vertexNormal(i);
    }

    Vec3Array vertexNormals() const
    {
      return {_vertexNormals, _vertexSize};
    }

    auto triangleCount() const
    {
      return _triangleSize;
    }

    auto& triangle(index_t i)
    {
      assert(i < _triangleSize);
      return _triangles[i];
    }

    auto& triangle(index_t i) const
    {
      return const_cast<Data*>(this)->triangle(i);
    }

    TriangleArray triangles() const
    {
      return {_triangles, _triangleSize};
    }

  private:
    index_t _vertexSize;
    index_t _triangleSize;
    vec3* _vertices;
    vec3* _vertexNormals{};
    Triangle* _triangles;

    Data(const Data&) = default;

    friend TriangleMesh;

  }; // Data

  TriangleMesh(Data&& data);

  auto& data() const
  {
    return _data;
  }

  bool hasVertexNormals() const
  {
    return _data._vertexNormals;
  }

  void computeVertexNormals();

  Bounds& bounds() const;
  void print(const char* label, FILE* file = stdout) const;

private:
  Data _data;
  mutable Bounds _bounds;

}; // TriangleMesh

ObjectPtr<TriangleMesh> readOBJ(const char* filename);

} // end namespace tcii::cg

#endif // __TriangleMesh_h

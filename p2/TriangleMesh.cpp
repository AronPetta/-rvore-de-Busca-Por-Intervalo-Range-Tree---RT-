// OVERVIEW: TriangleMesh.cpp
// ========
// Source file for triangle mesh.
//
// Author: Paulo Pagliosa
// Last revision: 06/07/2025

#include "TriangleMesh.h"
#include <cstring>

namespace tcii::cg
{ // begin namespace tcii::cg

namespace
{ // begin namespace

template <typename vec3>
inline auto&
operator += (vec3& a, const vec3& b)
{
  return a = a + b;
}

template <typename vec3>
inline auto&
normalize(vec3& v)
{
  return v = v.versor();
}

template <typename vec3>
inline auto
cross(const vec3& u, const vec3& v)
{
  const auto x = u.y * v.z - u.z * v.y;
  const auto y = u.z * v.x - u.x * v.z;
  const auto z = u.x * v.y - u.y * v.x;

  return vec3{x, y, z};
}

template <typename vec3>
inline auto
normal(const vec3& v0, const vec3& v1, const vec3& v2)
{
  return cross(v1 - v0, v2 - v0).versor();
}

template <typename vec3>
inline auto
normal(const vec3* v, const TriangleMesh::Triangle& t)
{
  return normal(v[t.i], v[t.j], v[t.k]);
}

template <typename Bounds>
auto empty(const Bounds& b)
{
  return b[0].x >= b[1].x || b[0].y >= b[1].y || b[0].z >= b[1].z;
}

} // end namespace

TriangleMesh::Data::Data(index_t vertexSize, index_t triangleSize):
  _vertexSize{vertexSize},
  _triangleSize{triangleSize}
{
  assert(vertexSize >= 3 && triangleSize >= 1);
  _vertices = new vec3[vertexSize];
  _triangles = new Triangle[triangleSize];
}

TriangleMesh::TriangleMesh(Data&& data):
  _data{data}
{
  data._vertices = nullptr;
  data._vertexNormals = nullptr;
  data._triangles = nullptr;
}

auto
TriangleMesh::bounds() const -> Bounds&
{
  if (empty(_bounds))
    for (index_t i{}; i < _data._vertexSize; i++)
      _bounds.inflate(_data._vertices[i]);
  return _bounds;
}

void
TriangleMesh::computeVertexNormals()
{
  auto nv = _data._vertexSize;

  if (!_data._vertexNormals)
    _data._vertexNormals = new vec3[nv];
  memset(_data._vertexNormals, 0, nv * sizeof(vec3));

  auto t = _data._triangles;

  for (index_t i{}; i < _data._triangleSize; ++i, ++t)
  {
    auto N = normal(_data._vertices, *t);

    _data._vertexNormals[t->i] += N;
    _data._vertexNormals[t->j] += N;
    _data._vertexNormals[t->k] += N;
  }
  for (index_t i = 0; i < nv; ++i)
    normalize(_data._vertexNormals[i]);
}

namespace
{ // begin namespace

template <typename vec3>
inline void
printv(const vec3& p, FILE* f)
{
  fprintf(f, "%g,%g,%g", p.x, p.y, p.z);
}

} // end namespace

void
TriangleMesh::print(const char* s, FILE* f) const
{
  fprintf(f, "%s mesh\n{\n", s);
  fprintf(f, "  vertices %d\n  {\n", _data._vertexSize);
  for (index_t i{}; i < _data._vertexSize; ++i)
  {
    fprintf(f, "    %d ", i);
    printv(_data._vertices[i], f);
    if (_data._vertexNormals != nullptr)
    {
      fputc('/', f);
      printv(_data._vertexNormals[i], f);
    }
    fputc('\n', f);
  }
  fprintf(f, "  }\n  triangles %d\n  {\n", _data._triangleSize);

  auto t = _data._triangles;

  for (index_t i{}; i < _data._triangleSize; ++i, ++t)
    fprintf(f, "    %d %d,%d,%d\n", i, t->i, t->j, t->k);
  fprintf(f, "  }\n}\n");
}

} // end namespace tcii::cg

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif // _MSC_VER

#include "TriangleMesh.h"
#include <filesystem>
#include <utility>

namespace tcii::cg
{ // begin namespace tcii::cg

namespace
{ // begin namespace

auto
readMeshSize(FILE* file)
{
  using index_t = TriangleMesh::index_t;

  constexpr auto maxSize = 256;
  index_t nv{};
  index_t nt{};
  char buffer[maxSize];

  while (char* line = fgets(buffer, maxSize, file))
    switch (*line++)
    {
      case 'v':
        if (*line == ' ')
          nv++;
        break;

      case 'f':
      {
        index_t v;
        index_t n;
        index_t t;
        index_t nfv{};

        for (;; ++nfv)
        {
          while (*line == ' ')
            line++;
          if (sscanf(line, "%d/%d/%d", &v, &t, &n) <= 0)
            break;
          while (*line && *line != ' ')
            ++line;
        }
        nt += nfv - 2;
        break;
      }
    }
  return std::pair{nv, nt};
}

void
readMeshData(FILE* file, TriangleMesh::Data& data)
{
  using index_t = TriangleMesh::index_t;

  constexpr auto maxSize = 256;
  index_t vid{};
  index_t tid{};
  char buffer[maxSize];

  while (char* line = fgets(buffer, maxSize, file))
    switch (*line++)
    {
      case 'v':
        if (*line == ' ')
        {
          float x;
          float y;
          float z;

          (void)sscanf(line + 1, "%f %f %f", &x, &y, &z);
          data.vertex(vid++) = {x, y, z};
        }
        break;

      case 'f':
      {
        index_t v;
        index_t n;
        index_t t;
        auto& triangle = data.triangle(tid);

        // This version reads vertex coordinates only and
        // ignores vertex texture coordinates and normals
        for (index_t i{};; ++i)
        {
          while (*line == ' ')
            line++;
          if (sscanf(line, "%d/%d/%d", &v, &t, &n) <= 0)
            break;
          if (i < 3)
            triangle[i] = v - 1;
          else
            data.triangle(++tid).set(triangle.i, triangle.k, v - 1);
          while (*line && *line != ' ')
            ++line;
        }
        ++tid;
        break;
      }
    }
}

} // end namespace

ObjectPtr<TriangleMesh>
readOBJ(const char* filename)
{
  FILE* file = fopen(filename, "r");

  if (file == nullptr)
    return nullptr;

  auto [nv, nt] = readMeshSize(file);
  TriangleMesh::Data data{nv, nt};

  rewind(file);
  printf("Reading Wavefront OBJ file %s...\n", filename);
  readMeshData(file, data);
  fclose(file);

  auto mesh = new TriangleMesh{std::move(data)};

  mesh->computeVertexNormals();
  return mesh;
}

} // end namespace tcii::cg

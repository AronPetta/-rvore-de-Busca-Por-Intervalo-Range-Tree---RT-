#include "Bounds3.h"
#include "RangeTree.h"
#include "Utils.h"
#include <cstdio>
#include <cstdlib>

using namespace tcii;

using Point = cg::Vec3f;
using PointArray = cg::Array<Point>;
using Bounds = cg::Bounds3f;

template <>
struct cg::PointTraits<Point>
{
  static constexpr size_t dim = 3;
  using Bounds = ::Bounds;
};

using PointSource = cg::PointSource<Point, PointArray>;
using RangeTree = cg::RangeTree<Point, PointArray>;

void
print(size_t id, const Point& p)
{
  printf("%05zu<%g,%g,%g>\n", id, p.x, p.y, p.z);
}

bool
printPoint(const PointArray& points, size_t pid)
{
  print(pid, points[pid]);
  return true;
}

void
queryTest(const PointArray& points, const Bounds& bounds)
{
  size_t np{};

  for (auto& p : points)
    if (bounds.contains(p))
      print(np++, p);
  printf("%zu points found\n", np);
}

int
main(int argc, char** argv)
{
  size_t np{100};
  cg::Bounds3f bounds;

  bounds.inflate({0, 0, 0});
  bounds.inflate({5, 5, 5});
  
  auto points = PointSource{}.random(np, bounds);

  bounds.setEmpty();
  bounds.inflate({1, 1, 1});
  bounds.inflate({3, 3, 3});
  puts("**Naive query");
  queryTest(points, bounds);

  RangeTree rt{points};

  rt.build();
  puts("**RT query");
  np = rt.query(bounds, printPoint);
  printf("%zu points found\n", np);
  puts("Press any key to exit...");
  (void)getchar();
  return 0;
}

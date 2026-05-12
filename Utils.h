#ifndef __Utils_h
#define __Utils_h

#include "PointTraits.h"
#include <random>

namespace tcii::cg
{ // begin namespace tcii::cg

template <typename P, typename A>
class PointSource
{
public:
  constexpr static auto D = point_dim_v<P>;

  using real = typename P::value_type;
  using Bounds = typename PointTraits<P>::Bounds;

  A random(size_t, const Bounds&) const;

}; // PointSource

template <typename P, typename A>
A
PointSource<P, A>::random(size_t n, const Bounds& bounds) const
{
  using URD = std::uniform_real_distribution<real>;

  std::random_device rd;
  std::mt19937 gen(rd());
  URD dists[D];

  for (size_t d = 0; d < D; ++d)
    dists[d] = URD{bounds[0][d], bounds[1][d]};

  A points(n);

  for (size_t i = 0; i < n; ++i)
  {
    auto& p = points[i];

    for (size_t d = 0; d < D; ++d)
      p[d] = dists[d](gen);
  }
  return points;
}

} // end namesapce tcii::cg

#endif // __Utils_h

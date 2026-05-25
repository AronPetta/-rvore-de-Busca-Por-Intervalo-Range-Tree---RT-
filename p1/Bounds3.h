#ifndef __Bounds3_h
#define __Bounds3_h

// OVERVIEW: Bounds3.h
// ========
// Class definition for generic array.
//
// Author: Paulo Pagliosa

#include "Vec3.h"

namespace tcii::cg
{ // begin namespace tcii::cg

template <size_t D, typename real> class Bounds;

template <typename real>
class Bounds<3, real>
{
public:
  using vec3 = Vec3<real>;

  Bounds()
  {
    setEmpty();
  }

  auto& operator [](size_t i) const
  {
    assert(i < 2);
    return (&_p1)[i];
  }

  auto& min() const
  {
    return _p1;
  }

  auto& max() const
  {
    return _p2;
  }

  bool contains(const vec3& p) const
  {
    if (p.x < _p1.x || p.x > _p2.x)
      return false;
    if (p.y < _p1.y || p.y > _p2.y)
      return false;
    if (p.z < _p1.z || p.z > _p2.z)
      return false;
    return true;
  }

  void setEmpty()
  {
    _p1.x = _p1.y = _p1.z = +std::numeric_limits<real>::max();
    _p2.x = _p2.y = _p2.z = -std::numeric_limits<real>::max();
  }

  void inflate(const vec3& p)
  {
    if (p.x < _p1.x)
      _p1.x = p.x;
    if (p.x > _p2.x)
      _p2.x = p.x;
    if (p.y < _p1.y)
      _p1.y = p.y;
    if (p.y > _p2.y)
      _p2.y = p.y;
    if (p.z < _p1.z)
      _p1.z = p.z;
    if (p.z > _p2.z)
      _p2.z = p.z;
  }

  void inflate(const Bounds& bounds)
  {
    inflate(bounds._p1);
    inflate(bounds._p2);
  }

private:
  vec3 _p1;
  vec3 _p2;

}; // Bounds3

template <typename real> using Bounds3 = Bounds<3, real>;

template <typename real>
std::ostream&
operator <<(std::ostream& os, const Bounds3<real>& b)
{
  os << "min" << b.min() << " max" << b.max();
  return os;
}

using Bounds3f = Bounds3<float>;

} // end namespace tcii::cg

#endif // __Bounds3_h
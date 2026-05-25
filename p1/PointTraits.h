#ifndef __PointTraits_h
#define __PointTraits_h

// OVERVIEW: PointTraits.h
// ========
// Class definition for generic array.
//
// Author: Paulo Pagliosa

#include <cstddef>

namespace tcii::cg
{ // begin namespace tcii::cg

template <typename P> struct PointTraits;

template <typename P> 
inline constexpr size_t point_dim_v = PointTraits<P>::dim;

} // end namesapce tcii::cg

#endif // __PointTraits_h

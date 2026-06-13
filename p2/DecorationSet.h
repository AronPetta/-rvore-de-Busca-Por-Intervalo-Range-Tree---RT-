#ifndef __DecorationSet_h
#define __DecorationSet_h

// OVERVIEW: DecorationSet.h
// ========
// Class definition for decoration set.
//
// Author: Paulo Pagliosa
// Last revision: 03/06/2026

#include "SoA.h"

//#define DEBUG

// The following macro is used in a class definition to circumvent
// the EBO bug with multiple empty base classes in MSVC as pointed
// out at https://en.cppreference.com/w/cpp/language/ebo.html
#ifdef _MSC_VER
#define EMPTY_BASES __declspec(empty_bases)
#else
#define EMPTY_BASES
#endif // _MSC_VER

namespace tcii::cg
{ // begin namespace tcii::cg

struct Allocator
{
  template <typename T>
  static T* allocate(size_t size)
  {
    auto ptr = new T[size];

#ifdef DEBUG
    printf("allocate(%zu):%p\n", size, ptr);
#endif // DEBUG
    return ptr;
  }

  template <typename T>
  static void free(T* ptr)
  {
#ifdef DEBUG
    printf("free(%p)\n", ptr);
#endif // DEBUG
    delete[] ptr;
  }

}; // Allocator

using index_t = unsigned;

template <typename... Fields>
using ElementSoA = SoA<Allocator, index_t, Fields...>;

template <typename T>
struct is_elementSoA: std::false_type {};

template <typename... Fields>
struct is_elementSoA<SoA<Allocator, index_t, Fields...>>: std::true_type {};

template <typename T>
constexpr bool is_elementSoA_v = is_elementSoA<T>::value;

template <typename T>
concept IsElementSoA = is_elementSoA_v<T>;


/////////////////////////////////////////////////////////////////////
//
// ElementAttributes: class for element attributes
// =================
template <typename T>
  requires IsElementSoA<T> || std::same_as<T, void>
class ElementAttributes
{
public:
  ElementAttributes(index_t size):
    _soa{size}
  {
    assert(size > 0);
  }

  ElementAttributes(ElementAttributes&&) noexcept = default;

  auto size() const
  {
    return _soa.size();
  }

  template <size_t I>
  auto& get(index_t i) const
  {
    return _soa.template get<I>(i);
  }

  template <size_t I, typename Field>
  void set(index_t i, Field&& field)
  {
    _soa.template get<I>(i) = std::forward<Field>(field);
  }

  template <typename... Fields>
  void set(index_t i, Fields&&... fields)
  {
    _soa.set(i, std::forward<Fields>(fields)...);
  }

private:
  T _soa;

}; // ElementAttributes

template <>
class ElementAttributes<void>
{
public:
  ElementAttributes(index_t size):
    _size{size}
  {
    assert(size > 0);
  }

  ElementAttributes(ElementAttributes&&) noexcept = default;

  auto size() const
  {
    return _size;
  }

private:
  index_t _size;

}; // ElementAttributes

template <size_t E, typename T>
class EMPTY_BASES ElementDecoration
{
public:
  ElementDecoration(index_t size):
    _attributes{size}
  {
    // do nothing
  }

  ElementDecoration(ElementDecoration&&) noexcept = default;

  template <typename U>
    requires (!std::same_as<T, U>)
  ElementDecoration(ElementDecoration<E, U>&& other):
    ElementDecoration{other.size()}
  {
    // do nothing
  }

  auto size() const
  {
    return _attributes.size();
  }

  auto& attributes() const
  {
    return _attributes;
  }

  auto& attributes()
  {
    return _attributes;
  }

private:
  ElementAttributes<T> _attributes;

}; // ElementDecoration

template <size_t E>
class ElementDecoration<E, void>: public ElementAttributes<void>
{
public:
  using ElementAttributes<void>::ElementAttributes;

  template <typename U>
    requires (!std::same_as<void, U>)
  ElementDecoration(ElementDecoration<E, U>&& other):
    ElementDecoration{other.size()}
  {
    // do nothing
  }

}; // ElementDecoration

template <size_t N, typename... Ts>
class DecorationSet_t {};

template <size_t N, typename T, typename... Ts>
class EMPTY_BASES DecorationSet_t<N, T, Ts...>:
  public DecorationSet_t<N + 1, Ts...>,
  public ElementDecoration<N, T>
{
public:
  using type = T;
  using Base = DecorationSet_t<N + 1, Ts...>;

  template <typename... S>
    requires (std::same_as<S, index_t> && ...)
  DecorationSet_t(index_t size, S... sizes):
    Base{sizes...},
    ElementDecoration<N, T>{size}
  {
    // do nothing
  }

  template <typename U, typename... Us>
  DecorationSet_t(DecorationSet_t<N, U, Us...>&& other):
    Base{static_cast<DecorationSet_t<N + 1, Us...>&&>(other)},
    ElementDecoration<N, T>{std::move(other)}
  {
    // do nothing
  }

  template <size_t E>
  friend auto& attributes(DecorationSet_t<N, T, Ts...>& ds) noexcept
  {
    if constexpr (E == N)
    {
      ASSERT_IS_NOT_VOID(T, "Element has no attributes");
      return ds.ElementDecoration<N, T>::attributes();
    }
    else
    {
      // Enforce bounds checking
      static_assert(sizeof...(Ts), "Invalid element id");
      return attributes<E>(static_cast<Base&>(ds));
    }
  }

}; // DecorationSet_t

template <size_t E, size_t N, typename... Ts>
inline const auto&
attributes(const DecorationSet_t<N, Ts...>& ds) noexcept
{
  return attributes<E>(const_cast<DecorationSet_t<N, Ts...>&>(ds));
}

template <size_t E, size_t I, size_t N, typename... Ts>
inline const auto&
get(const DecorationSet_t<N, Ts...>& ds, index_t i)
{
  return attributes<E>(ds).template get<I>(i);
}

template <size_t E, size_t I, typename Field, size_t N, typename... Ts>
inline void
set(DecorationSet_t<N, Ts...>& ds, index_t i, Field&& field)
{
  return attributes<E>(ds).template set<I>(i, std::forward<Field>(field));
}

template <typename... Ts>
using DecorationSet = DecorationSet_t<0, Ts...>;

} // end namespace tcii::cg

#endif // __DecorationSet_h

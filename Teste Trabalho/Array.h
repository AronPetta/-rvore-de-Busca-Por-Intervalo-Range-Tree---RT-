#ifndef __Array_h
#define __Array_h

// OVERVIEW: Array.h
// ========
// Class definition for generic array.
//
// Author: Paulo Pagliosa
// Last revision: 01/05/2026

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <utility>

namespace tcii::cg
{ // begin namespace tcii::cg


/////////////////////////////////////////////////////////////////////
//
// ArrayAllocator: standard array allocator class
// ==============
class ArrayAllocator
{
public:
  template <typename T>
  static T* allocate(size_t count)
  {
    return new T[count];
  }

  template <typename T>
  static void free(T* ptr)
  {
    delete []ptr;
  }

}; // ArrayAllocator


/////////////////////////////////////////////////////////////////////
//
// Array: array class
// =====
template <typename T, typename Allocator = ArrayAllocator>
class Array
{
public:
  using value_type = T;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using reference = value_type&;
  using const_reference = const value_type&;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using difference_type = std::ptrdiff_t;

  Array() = default;

  Array(size_t size):
    _data{Allocator::template allocate<T>(size)},
    _size{size}
  {
    // do nothing
  }

  template <typename It>
  Array(It first, It last):
    Array(std::distance(first, last))
  {
    for (size_t i = 0; first < last;)
      _data[i++] = *first++;
  }

  Array(Array&& other) noexcept:
    _data{std::exchange(other._data, nullptr)},
    _size{std::exchange(other._size, 0)}
  {
    // do nothing
  }

  Array(std::initializer_list<T> list):
    _data{Allocator::template allocate<T>(list.size())}
  {
    for (const auto& item : list)
      _data[_size++] = item;
  }

  ~Array()
  {
    Allocator::template free<T>(_data);
  }

  auto& operator =(Array&& other) noexcept
  {
    if (this != &other)
    {
      this->~Array();
      _data = std::exchange(other._data, nullptr);
      _size = std::exchange(other._size, 0);
    }
    return *this;
  }

  size_t size() const
  {
    return _size;
  }

  auto data() const
  {
    return _data;
  }

  const auto& operator [](size_t index) const
  {
    assert(index < this->size());
    return _data[index];
  }

  auto begin() const
  {
    return _data;
  }

  auto end() const
  {
    return _data + _size;
  }

  auto data()
  {
    return _data;
  }

  auto& operator [](size_t index)
  {
    assert(index < this->size());
    return this->_data[index];
  }

  auto begin()
  {
    return _data;
  }

  auto end()
  {
    return _data + _size;
  }

protected:
  T* _data{};
  size_t _size{};

  Array(const Array&) = delete;
  Array& operator =(const Array&) = delete;

}; // Array

} // end namespace tcii::cg

#endif // __Array_h

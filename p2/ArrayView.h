#ifndef __ArrayView_h
#define __ArrayView_h

#include <cassert>

namespace tcii::cg
{ // begin namespace tcii::cg

template <typename T>
class ArrayView
{
public:
  using value_type = T;

  ArrayView() = default;

  ArrayView(const T* data, size_t size):
    _data{data},
    _size{size}
  {
    // do nothing
  }

  auto data() const
  {
    return _data;
  }

  auto size() const
  {
    return _size;
  }

  auto& operator [](size_t i) const
  {
    assert(i < _size);
    return _data[i];
  }

  bool empty() const
  {
    return !_data;
  }

private:
  const T* _data{};
  size_t _size{};

}; // ArrayView

} // end namespace tcii::cg

#endif // __ArrayView_h

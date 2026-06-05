#ifndef __AVL_h
#define __AVL_h

#include <functional>
#include <initializer_list>
#include <cstdint>
#include <utility>

namespace tcii::avl
{ // begin namespace tcii::avl

struct TreeNodeBase
{
  TreeNodeBase* _parent;
  TreeNodeBase* _childL{};
  TreeNodeBase* _childR{};
  int _height{};

  TreeNodeBase(TreeNodeBase* parent):
    _parent{parent}
  {
    // do nothing
  }

protected:
  ~TreeNodeBase() = default;

}; // TreeNodeBase

struct AVLHelper
{
  static auto height(const TreeNodeBase* node)
  {
    return node ? node->_height : 0;
  }

  static auto balanceFactor(const TreeNodeBase* node)
  {
    return node ? height(node->_childR) - height(node->_childL) : 0;
  }

  static void updateHeight(TreeNodeBase* node)
  {
    if (node)
    {
      auto hl = height(node->_childL);
      auto hr = height(node->_childR);

      node->_height = (hl > hr ? hl : hr) + 1;
    }
  }

  static void rotateL(TreeNodeBase*&);
  static void rotateR(TreeNodeBase*&);
  static void balance(TreeNodeBase*&);

}; // AVLHelper

struct TreeIteratorBase
{
  bool operator ==(const TreeIteratorBase& other) const
  {
    return _node == other._node;
  }

  bool operator !=(const TreeIteratorBase& other) const
  {
    return !operator ==(other);
  }

protected:
  TreeNodeBase* _node;

  TreeIteratorBase(TreeNodeBase* node):
    _node{node}
  {
    // do nothing
  }

  void increment();
  void decrement();

}; // TreeIteratorBase

template <typename T>
class TreeNode: public TreeNodeBase
{
public:
  T _value;

  TreeNode(const T& value, TreeNodeBase* parent):
    TreeNodeBase{parent},
    _value{value}
  {
    // do nothing
  }

  ~TreeNode()
  {
    delete (TreeNode*)_childL;
    delete (TreeNode*)_childR;
  }

}; // TreeNode

template <typename T>
class TreeIterator: public TreeIteratorBase
{
public:
  TreeIterator(TreeNode<T>* node):
    TreeIteratorBase{node}
  {
    // do nothing
  }

  auto& operator *() const
  { 
    return ((TreeNode<T>*)_node)->_value;
  }

  auto operator ->() const
  {
    return &operator *();
  }

  auto& operator ++()
  {
    increment();
    return *this;
  }

  auto operator ++(int)
  {
    auto temp = *this;
 
    increment();
    return temp;
  }
    
  // INSERT ABAIXO
  auto& operator --()
{
  decrement();
  return *this;
}

auto operator --(int)
{
  auto temp = *this;

  decrement();
  return temp;
}
// INSERT ACIMA
}; // TreeIterator

template <typename T, typename C = std::less<T>>
class Tree
{
public:
  using iterator = TreeIterator<T>;
  using IterFunc = void(const T&);

  Tree(C comp = C{}):
    _comp{comp}
  {
    // do nothing
  }

  ~Tree()
  {
    clear();
  }

  void clear()
  {
    if (_root)
    {
      delete _root;
      _root = nullptr;
      _nodeCount = 0;
    }
  }

  std::pair<iterator, bool> insert(const T&);

  void insert(std::initializer_list<T> list)
  {
    for (const auto& value : list)
      insert(value);
  }

  auto size() const
  {
    return _nodeCount;
  }

  auto empty() const
  {
    return !_root;
  }

  void iterate(IterFunc func) const
  {
    iterate(_root, func);
  }

  auto height() const
  {
    return AVLHelper::height(_root);
  }

  iterator begin() const;

  auto end() const
  {
    return iterator{nullptr};
  }

  iterator find(const T&) const;

  auto contains(const T& value) const
  {
    return find(value) != end();
  }
  
  // INSERT ABAIXO
  auto rbegin() const
{
  auto node = _root;

  if (node)
    while (node->_childR)
      node = (Node*)node->_childR;

  return iterator{node};
}

auto rend() const
{
  return iterator{nullptr};
}
// INSERT ENCIMA

private:
  using Node = TreeNode<T>;

  Node* _root{};
  unsigned _nodeCount{};
  C _comp;

  static Node* insert(Node*, TreeNodeBase*, const T&, Node*&, C&);
  static void iterate(Node*, IterFunc);

}; // Tree

template <typename T, typename C>
auto
Tree<T, C>::insert(const T& value) -> std::pair<iterator, bool>
{
  Node* valueNode;
  auto root = insert(_root, nullptr, value, valueNode, _comp);
  auto success = bool(root);

  if (success)
  {
    _root = root;
    _nodeCount++;
  }
  return {valueNode, success};
}

template <typename T, typename C>
auto
Tree<T, C>::begin() const -> iterator
{
  TreeNodeBase* node{_root};

  if (node)
    while (node->_childL)
      node = node->_childL;
  return iterator{(Node*)node};
}

template <typename T, typename C>
auto
Tree<T, C>::find(const T& value) const -> iterator
{
  // INSERT ABAIXO
  template <typename T, typename C>
auto
Tree<T, C>::find(const T& value) const -> iterator
{
  auto node = _root;

  while (node)
  {
    if (_comp(value, node->_value))
      node = (Node*)node->_childL;
    else if (_comp(node->_value, value))
      node = (Node*)node->_childR;
    else
      return iterator{node};
  }

  return end();
}
  // INSERT ENCIMA
  return end();
}

template <typename T, typename C>
TreeNode<T>*
Tree<T, C>::insert(Node* node,
  TreeNodeBase* parent,
  const T& value,
  Node*& valueNode,
  C& comp)
{
  if (!node)
    return valueNode = new Node{value, parent};
  if (comp(value, node->_value))
  {
    if (auto l = insert((Node*)node->_childL, node, value, valueNode, comp))
      node->_childL = l;
    else
      return nullptr;
  }
  else if (comp(node->_value, value))
  {
    if (auto r = insert((Node*)node->_childR, node, value, valueNode, comp))
      node->_childR = r;
    else
      return nullptr;
  }
  else
    return (valueNode = node), nullptr;

  TreeNodeBase* temp{node};

  AVLHelper::balance(temp);
  return (Node*)temp;
}

template <typename T, typename C>
void
Tree<T, C>::iterate(Node* node, IterFunc func)
{
  if (node)
  {
    iterate((Node*)node->_childL, func);
    func(node->_value);
    iterate((Node*)node->_childR, func);
  }
}

} // end namespace tcii::avl

#endif // __AVL_h

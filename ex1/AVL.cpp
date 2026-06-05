#include "AVL.h"
#include <algorithm>
#include <cassert>

namespace tcii::avl
{ // begin namespace tcii::avl

void
TreeIteratorBase::increment()
{
  if (_node->_childR)
    for (_node = _node->_childR; _node->_childL;)
      _node = _node->_childL;
  else
  {
    auto p = _node->_parent;

    while (p && _node == p->_childR)
    {
      _node = p;
      p = p->_parent;
    }
    _node = p;
  }
}

void
TreeIteratorBase::decrement(){ // Decrementação
  if (_node->_childL){
    for (_node = _node->_childL; _node->_childR;){
      _node = _node->_childR;
    }
  }else{
    auto p = _node->_parent;

    while (p && _node == p->_childL){
      _node = p;
      p = p->_parent;
    }
    _node = p;
  }
}

void
AVLHelper::rotateL(TreeNodeBase*& a)
{ 
  auto b = a->_childR;

  a->_childR = b->_childL;
  if (b->_childL) b->_childL->_parent = a;
  b->_parent = a->_parent;
  b->_childL = a;
  a->_parent = b;
  updateHeight(a);
  updateHeight(b);
  a = b;
}

void
AVLHelper::rotateR(TreeNodeBase*& a)
{
  auto b = a->_childL;

  a->_childL = b->_childR;
  if (b->_childR) b->_childR->_parent = a;
  b->_parent = a->_parent;
  b->_childR = a;
  a->_parent = b;
  updateHeight(a);
  updateHeight(b);
  a = b;
}

void
AVLHelper::balance(TreeNodeBase*& node)
{
  if (!node)
    return;
  updateHeight(node);
  if (auto bf = balanceFactor(node); bf > 1)
  {
    if (balanceFactor(node->_childR) < 0)
      rotateR(node->_childR);
    rotateL(node);
  }
  else if (bf < -1)
  {
    if (balanceFactor(node->_childL) > 0)
      rotateL(node->_childL);
    rotateR(node);
  }
}

} // end namespace tcii::avl

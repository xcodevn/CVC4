/*
 * expr_manager_scope.h
 *
 *  Created on: Apr 7, 2010
 *      Author: dejan
 */

#ifndef __CVC4__EXPR_MANAGER_SCOPE_H
#define __CVC4__EXPR_MANAGER_SCOPE_H

#include "expr/expr.h"
#include "expr/node_manager.h"

namespace CVC4 {

/**
 * Creates a <code>NodeManagerScope</code> with the underlying
 * <code>NodeManager</code> of a given <code>Expr</code> or
 * <code>ExprManager</code>.  The <code>NodeManagerScope</code> is
 * destroyed when the <code>ExprManagerScope</code> is destroyed. See
 * <code>NodeManagerScope</code> for more information.
 */
// NOTE: Here, it seems ExprManagerScope is redundant, since we have
// NodeManagerScope already.  However, without this class, we'd need
// Expr to be a friend of ExprManager, because in the implementation
// of Expr functions, it needs to set the current NodeManager
// correctly (and to do that it needs access to
// ExprManager::getNodeManager()).  So, we make ExprManagerScope a
// friend of ExprManager's, since its implementation is simple to
// read and understand (and verify that it doesn't do any mischief).
//
// ExprManager::getNodeManager() can't just be made public, since
// ExprManager is exposed to clients of the library and NodeManager is
// not.  Similarly, ExprManagerScope shouldn't go in expr_manager.h,
// since that's a public header.
class ExprManagerScope {
  NodeManagerScope d_nms;
public:
  inline ExprManagerScope(const Expr& e) :
    d_nms(e.getExprManager() == NULL
          ? NodeManager::currentNM()
          : e.getExprManager()->getNodeManager()) {
  }
  inline ExprManagerScope(const ExprManager& exprManager) :
    d_nms(exprManager.getNodeManager()) {
  }
};

}


#endif /* __CVC4__EXPR_MANAGER_SCOPE_H */
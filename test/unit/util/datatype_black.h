/*********************                                                        */
/*! \file datatype_black.h
 ** \verbatim
 ** Original author: Morgan Deters
 ** Major contributors: none
 ** Minor contributors (to current version): Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2013  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Black box testing of CVC4::Datatype
 **
 ** Black box testing of CVC4::Datatype.
 **/

#include <cxxtest/TestSuite.h>
#include <sstream>

#include "util/datatype.h"

#include "expr/expr.h"
#include "expr/expr_manager.h"

using namespace CVC4;
using namespace std;

class DatatypeBlack : public CxxTest::TestSuite {

  ExprManager* d_em;

public:

  void setUp() {
    d_em = new ExprManager();
    Debug.on("datatypes");
    Debug.on("groundterms");
  }

  void tearDown() {
    delete d_em;
  }

  void testEnumeration() {
    Datatype colors("colors");

    DatatypeConstructor yellow("yellow", "is_yellow");
    DatatypeConstructor blue("blue", "is_blue");
    DatatypeConstructor green("green", "is_green");
    DatatypeConstructor red("red", "is_red");

    colors.addConstructor(yellow);
    colors.addConstructor(blue);
    colors.addConstructor(green);
    colors.addConstructor(red);

    Debug("datatypes") << colors << std::endl;
    DatatypeType colorsType = d_em->mkDatatypeType(colors);
    Debug("datatypes") << colorsType << std::endl;

    Expr ctor = colorsType.getDatatype()[1].getConstructor();
    Expr apply = d_em->mkExpr(kind::APPLY_CONSTRUCTOR, ctor);
    Debug("datatypes") << apply << std::endl;

    const Datatype& colorsDT = colorsType.getDatatype();
    TS_ASSERT(colorsDT.getConstructor("blue") == ctor);
    TS_ASSERT(colorsDT["blue"].getConstructor() == ctor);
    TS_ASSERT_THROWS(colorsDT["blue"].getSelector("foo"), IllegalArgumentException);
    TS_ASSERT_THROWS(colorsDT["blue"]["foo"], IllegalArgumentException);

    TS_ASSERT(colorsType.getDatatype().isFinite());
    TS_ASSERT(colorsType.getDatatype().getCardinality().compare(4) == Cardinality::EQUAL);
    TS_ASSERT(ctor.getType().getCardinality().compare(1) == Cardinality::EQUAL);
    TS_ASSERT(colorsType.getDatatype().isWellFounded());
    Debug("groundterms") << "ground term of " << colorsType.getDatatype().getName() << endl
                         << "  is " << colorsType.mkGroundTerm() << endl;
    TS_ASSERT(colorsType.mkGroundTerm().getType() == colorsType);
    // all ctors should be well-founded too
    for(Datatype::const_iterator i = colorsType.getDatatype().begin(),
          i_end = colorsType.getDatatype().end();
        i != i_end;
        ++i) {
      TS_ASSERT((*i).isWellFounded());
      Debug("groundterms") << "ground term of " << *i << endl
                           << "  is " << (*i).mkGroundTerm( colorsType ) << endl;
      TS_ASSERT((*i).mkGroundTerm( colorsType ).getType() == colorsType);
    }
  }

  void testNat() {
    Datatype nat("nat");

    DatatypeConstructor succ("succ", "is_succ");
    succ.addArg("pred", DatatypeSelfType());
    nat.addConstructor(succ);

    DatatypeConstructor zero("zero", "is_zero");
    nat.addConstructor(zero);

    Debug("datatypes") << nat << std::endl;
    DatatypeType natType = d_em->mkDatatypeType(nat);
    Debug("datatypes") << natType << std::endl;

    Expr ctor = natType.getDatatype()[1].getConstructor();
    Expr apply = d_em->mkExpr(kind::APPLY_CONSTRUCTOR, ctor);
    Debug("datatypes") << apply << std::endl;

    TS_ASSERT(! natType.getDatatype().isFinite());
    TS_ASSERT(natType.getDatatype().getCardinality().compare(Cardinality::INTEGERS) == Cardinality::EQUAL);
    TS_ASSERT(natType.getDatatype().isWellFounded());
    Debug("groundterms") << "ground term of " << natType.getDatatype().getName() << endl
                         << "  is " << natType.mkGroundTerm() << endl;
    TS_ASSERT(natType.mkGroundTerm().getType() == natType);
    // all ctors should be well-founded too
    for(Datatype::const_iterator i = natType.getDatatype().begin(),
          i_end = natType.getDatatype().end();
        i != i_end;
        ++i) {
      Debug("datatypes") << "checking " << (*i).getName() << endl;
      TS_ASSERT((*i).isWellFounded());
      Debug("groundterms") << "ground term of " << *i << endl
                           << "  is " << (*i).mkGroundTerm( natType ) << endl;
      TS_ASSERT((*i).mkGroundTerm( natType ).getType() == natType);
    }
  }

  void testTree() {
    Datatype tree("tree");
    Type integerType = d_em->integerType();

    DatatypeConstructor node("node", "is_node");
    node.addArg("left", DatatypeSelfType());
    node.addArg("right", DatatypeSelfType());
    tree.addConstructor(node);

    DatatypeConstructor leaf("leaf", "is_leaf");
    leaf.addArg("leaf", integerType);
    tree.addConstructor(leaf);

    Debug("datatypes") << tree << std::endl;
    DatatypeType treeType = d_em->mkDatatypeType(tree);
    Debug("datatypes") << treeType << std::endl;

    Expr ctor = treeType.getDatatype()[1].getConstructor();
    TS_ASSERT(treeType.getConstructor("leaf") == ctor);
    TS_ASSERT(treeType.getConstructor("leaf") == ctor);
    TS_ASSERT_THROWS(treeType.getConstructor("leff"), IllegalArgumentException);

    TS_ASSERT(! treeType.getDatatype().isFinite());
    TS_ASSERT(treeType.getDatatype().getCardinality().compare(Cardinality::INTEGERS) == Cardinality::EQUAL);
    TS_ASSERT(treeType.getDatatype().isWellFounded());
    Debug("groundterms") << "ground term of " << treeType.getDatatype().getName() << endl
                         << "  is " << treeType.mkGroundTerm() << endl;
    TS_ASSERT(treeType.mkGroundTerm().getType() == treeType);
    // all ctors should be well-founded too
    for(Datatype::const_iterator i = treeType.getDatatype().begin(),
          i_end = treeType.getDatatype().end();
        i != i_end;
        ++i) {
      TS_ASSERT((*i).isWellFounded());
      Debug("groundterms") << "ground term of " << *i << endl
                           << "  is " << (*i).mkGroundTerm( treeType ) << endl;
      TS_ASSERT((*i).mkGroundTerm( treeType ).getType() == treeType);
    }
  }

  void testListInt() {
    Datatype list("list");
    Type integerType = d_em->integerType();

    DatatypeConstructor cons("cons", "is_cons");
    cons.addArg("car", integerType);
    cons.addArg("cdr", DatatypeSelfType());
    list.addConstructor(cons);

    DatatypeConstructor nil("nil", "is_nil");
    list.addConstructor(nil);

    Debug("datatypes") << list << std::endl;
    DatatypeType listType = d_em->mkDatatypeType(list);
    Debug("datatypes") << listType << std::endl;

    TS_ASSERT(! listType.getDatatype().isFinite());
    TS_ASSERT(listType.getDatatype().getCardinality().compare(Cardinality::INTEGERS) == Cardinality::EQUAL);
    TS_ASSERT(listType.getDatatype().isWellFounded());
    Debug("groundterms") << "ground term of " << listType.getDatatype().getName() << endl
                         << "  is " << listType.mkGroundTerm() << endl;
    TS_ASSERT(listType.mkGroundTerm().getType() == listType);
    // all ctors should be well-founded too
    for(Datatype::const_iterator i = listType.getDatatype().begin(),
          i_end = listType.getDatatype().end();
        i != i_end;
        ++i) {
      TS_ASSERT((*i).isWellFounded());
      Debug("groundterms") << "ground term of " << *i << endl
                           << "  is " << (*i).mkGroundTerm( listType ) << endl;
      TS_ASSERT((*i).mkGroundTerm( listType ).getType() == listType);
    }
  }

  void testListReal() {
    Datatype list("list");
    Type realType = d_em->realType();

    DatatypeConstructor cons("cons", "is_cons");
    cons.addArg("car", realType);
    cons.addArg("cdr", DatatypeSelfType());
    list.addConstructor(cons);

    DatatypeConstructor nil("nil", "is_nil");
    list.addConstructor(nil);

    Debug("datatypes") << list << std::endl;
    DatatypeType listType = d_em->mkDatatypeType(list);
    Debug("datatypes") << listType << std::endl;

    TS_ASSERT(! listType.getDatatype().isFinite());
    TS_ASSERT(listType.getDatatype().getCardinality().compare(Cardinality::REALS) == Cardinality::EQUAL);
    TS_ASSERT(listType.getDatatype().isWellFounded());
    Debug("groundterms") << "ground term of " << listType.getDatatype().getName() << endl
                         << "  is " << listType.mkGroundTerm() << endl;
    TS_ASSERT(listType.mkGroundTerm().getType() == listType);
    // all ctors should be well-founded too
    for(Datatype::const_iterator i = listType.getDatatype().begin(),
          i_end = listType.getDatatype().end();
        i != i_end;
        ++i) {
      TS_ASSERT((*i).isWellFounded());
      Debug("groundterms") << "ground term of " << *i << endl
                           << "  is " << (*i).mkGroundTerm( listType ) << endl;
      TS_ASSERT((*i).mkGroundTerm( listType ).getType() == listType);
    }
  }

  void testListBoolean() {
    Datatype list("list");
    Type booleanType = d_em->booleanType();

    DatatypeConstructor cons("cons", "is_cons");
    cons.addArg("car", booleanType);
    cons.addArg("cdr", DatatypeSelfType());
    list.addConstructor(cons);

    DatatypeConstructor nil("nil", "is_nil");
    list.addConstructor(nil);

    Debug("datatypes") << list << std::endl;
    DatatypeType listType = d_em->mkDatatypeType(list);
    Debug("datatypes") << listType << std::endl;

    TS_ASSERT(! listType.getDatatype().isFinite());
    TS_ASSERT(listType.getDatatype().getCardinality().compare(Cardinality::INTEGERS) == Cardinality::EQUAL);
    TS_ASSERT(listType.getDatatype().isWellFounded());
    Debug("groundterms") << "ground term of " << listType.getDatatype().getName() << endl
                         << "  is " << listType.mkGroundTerm() << endl;
    TS_ASSERT(listType.mkGroundTerm().getType() == listType);
    // all ctors should be well-founded too
    for(Datatype::const_iterator i = listType.getDatatype().begin(),
          i_end = listType.getDatatype().end();
        i != i_end;
        ++i) {
      TS_ASSERT((*i).isWellFounded());
      Debug("groundterms") << "ground term of " << *i << endl
                           << "  is " << (*i).mkGroundTerm( listType ) << endl;
      TS_ASSERT((*i).mkGroundTerm( listType ).getType() == listType);
    }
  }

  void testMutualListTrees() {
    /* Create two mutual datatypes corresponding to this definition
     * block:
     *
     *   DATATYPE
     *     tree = node(left: tree, right: tree) | leaf(list),
     *     list = cons(car: tree, cdr: list) | nil
     *   END;
     */
    Datatype tree("tree");
    DatatypeConstructor node("node", "is_node");
    node.addArg("left", DatatypeSelfType());
    node.addArg("right", DatatypeSelfType());
    tree.addConstructor(node);

    DatatypeConstructor leaf("leaf", "is_leaf");
    leaf.addArg("leaf", DatatypeUnresolvedType("list"));
    tree.addConstructor(leaf);

    Debug("datatypes") << tree << std::endl;

    Datatype list("list");
    DatatypeConstructor cons("cons", "is_cons");
    cons.addArg("car", DatatypeUnresolvedType("tree"));
    cons.addArg("cdr", DatatypeSelfType());
    list.addConstructor(cons);

    DatatypeConstructor nil("nil", "is_nil");
    list.addConstructor(nil);

    Debug("datatypes") << list << std::endl;

    TS_ASSERT(! tree.isResolved());
    TS_ASSERT(! node.isResolved());
    TS_ASSERT(! leaf.isResolved());
    TS_ASSERT(! list.isResolved());
    TS_ASSERT(! cons.isResolved());
    TS_ASSERT(! nil.isResolved());

    vector<Datatype> dts;
    dts.push_back(tree);
    dts.push_back(list);
    vector<DatatypeType> dtts = d_em->mkMutualDatatypeTypes(dts);

    TS_ASSERT(dtts[0].getDatatype().isResolved());
    TS_ASSERT(dtts[1].getDatatype().isResolved());

    TS_ASSERT(! dtts[0].getDatatype().isFinite());
    TS_ASSERT(dtts[0].getDatatype().getCardinality().compare(Cardinality::INTEGERS) == Cardinality::EQUAL);
    TS_ASSERT(dtts[0].getDatatype().isWellFounded());
    Debug("groundterms") << "ground term of " << dtts[0].getDatatype().getName() << endl
                         << "  is " << dtts[0].mkGroundTerm() << endl;
    TS_ASSERT(dtts[0].mkGroundTerm().getType() == dtts[0]);
    // all ctors should be well-founded too
    for(Datatype::const_iterator i = dtts[0].getDatatype().begin(),
          i_end = dtts[0].getDatatype().end();
        i != i_end;
        ++i) {
      TS_ASSERT((*i).isWellFounded());
      Debug("groundterms") << "ground term of " << *i << endl
                           << "  is " << (*i).mkGroundTerm( dtts[0] ) << endl;
      TS_ASSERT((*i).mkGroundTerm( dtts[0] ).getType() == dtts[0]);
    }

    TS_ASSERT(! dtts[1].getDatatype().isFinite());
    TS_ASSERT(dtts[1].getDatatype().getCardinality().compare(Cardinality::INTEGERS) == Cardinality::EQUAL);
    TS_ASSERT(dtts[1].getDatatype().isWellFounded());
    Debug("groundterms") << "ground term of " << dtts[1].getDatatype().getName() << endl
                         << "  is " << dtts[1].mkGroundTerm() << endl;
    TS_ASSERT(dtts[1].mkGroundTerm().getType() == dtts[1]);
    // all ctors should be well-founded too
    for(Datatype::const_iterator i = dtts[1].getDatatype().begin(),
          i_end = dtts[1].getDatatype().end();
        i != i_end;
        ++i) {
      TS_ASSERT((*i).isWellFounded());
      Debug("groundterms") << "ground term of " << *i << endl
                           << "  is " << (*i).mkGroundTerm( dtts[1] ) << endl;
      TS_ASSERT((*i).mkGroundTerm( dtts[1] ).getType() == dtts[1]);
    }

    // add another constructor to list datatype resulting in an
    // "otherNil-list"
    DatatypeConstructor otherNil("otherNil", "is_otherNil");
    dts[1].addConstructor(otherNil);

    // remake the types
    vector<DatatypeType> dtts2 = d_em->mkMutualDatatypeTypes(dts);

    TS_ASSERT_DIFFERS(dtts, dtts2);
    TS_ASSERT_DIFFERS(dtts[1], dtts2[1]);

    // tree is also different because it's a tree of otherNil-lists
    TS_ASSERT_DIFFERS(dtts[0], dtts2[0]);

    TS_ASSERT(! dtts2[0].getDatatype().isFinite());
    TS_ASSERT(dtts2[0].getDatatype().getCardinality().compare(Cardinality::INTEGERS) == Cardinality::EQUAL);
    TS_ASSERT(dtts2[0].getDatatype().isWellFounded());
    Debug("groundterms") << "ground term of " << dtts2[0].getDatatype().getName() << endl
                         << "  is " << dtts2[0].mkGroundTerm() << endl;
    TS_ASSERT(dtts2[0].mkGroundTerm().getType() == dtts2[0]);
    // all ctors should be well-founded too
    for(Datatype::const_iterator i = dtts2[0].getDatatype().begin(),
          i_end = dtts2[0].getDatatype().end();
        i != i_end;
        ++i) {
      TS_ASSERT((*i).isWellFounded());
      Debug("groundterms") << "ground term of " << *i << endl
                           << "  is " << (*i).mkGroundTerm( dtts2[0] ) << endl;
      TS_ASSERT((*i).mkGroundTerm( dtts2[0] ).getType() == dtts2[0]);
    }

    TS_ASSERT(! dtts2[1].getDatatype().isFinite());
    TS_ASSERT(dtts2[1].getDatatype().getCardinality().compare(Cardinality::INTEGERS) == Cardinality::EQUAL);
    TS_ASSERT(dtts2[1].getDatatype().isWellFounded());
    Debug("groundterms") << "ground term of " << dtts2[1].getDatatype().getName() << endl
                         << "  is " << dtts2[1].mkGroundTerm() << endl;
    TS_ASSERT(dtts2[1].mkGroundTerm().getType() == dtts2[1]);
    // all ctors should be well-founded too
    for(Datatype::const_iterator i = dtts2[1].getDatatype().begin(),
          i_end = dtts2[1].getDatatype().end();
        i != i_end;
        ++i) {
      TS_ASSERT((*i).isWellFounded());
      Debug("groundterms") << "ground term of " << *i << endl
                           << "  is " << (*i).mkGroundTerm( dtts2[1] ) << endl;
      TS_ASSERT((*i).mkGroundTerm( dtts2[1] ).getType() == dtts2[1]);
    }
  }

  void testNotSoWellFounded() {
    Datatype tree("tree");

    DatatypeConstructor node("node", "is_node");
    node.addArg("left", DatatypeSelfType());
    node.addArg("right", DatatypeSelfType());
    tree.addConstructor(node);

    Debug("datatypes") << tree << std::endl;
    DatatypeType treeType = d_em->mkDatatypeType(tree);
    Debug("datatypes") << treeType << std::endl;

    TS_ASSERT(! treeType.getDatatype().isFinite());
    TS_ASSERT(treeType.getDatatype().getCardinality().compare(Cardinality::INTEGERS) == Cardinality::EQUAL);
    TS_ASSERT(! treeType.getDatatype().isWellFounded());
    TS_ASSERT_THROWS_ANYTHING( treeType.mkGroundTerm() );
    TS_ASSERT_THROWS_ANYTHING( treeType.getDatatype().mkGroundTerm( treeType ) );
    // all ctors should be not-well-founded either
    for(Datatype::const_iterator i = treeType.getDatatype().begin(),
          i_end = treeType.getDatatype().end();
        i != i_end;
        ++i) {
      TS_ASSERT(! (*i).isWellFounded());
      TS_ASSERT_THROWS_ANYTHING( (*i).mkGroundTerm( treeType ) );
    }
  }

};/* class DatatypeBlack */

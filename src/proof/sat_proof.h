/*********************                                                        */
/*! \file sat_proof.h
 ** \verbatim
 ** Original author: Liana Hadarean
 ** Major contributors: Morgan Deters
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2013  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Resolution proof 
 **
 ** Resolution proof
 **/

#include "cvc4_private.h"

#ifndef __CVC4__SAT__PROOF_H
#define __CVC4__SAT__PROOF_H

#include <iostream>
#include <stdint.h>
#include <vector>
#include <set>
#include <ext/hash_map>
#include <ext/hash_set>
#include <sstream>
#include "expr/expr.h"


namespace Minisat {
  class Solver;
  typedef uint32_t CRef;
}/* Minisat namespace */

#include "prop/minisat/core/SolverTypes.h"
#include "util/proof.h"

namespace std {
  using namespace __gnu_cxx;
}/* std namespace */

namespace CVC4 {

/** 
 * Helper debugging functions
 */
void printDebug(::Minisat::Lit l);
void printDebug(::Minisat::Clause& c); 

typedef int ClauseId;
struct ResStep {
  ::Minisat::Lit lit;
  ClauseId id;
  bool sign;
  ResStep(::Minisat::Lit l, ClauseId i, bool s) :
    lit(l),
    id(i),
    sign(s)
  {}
};/* struct ResStep */

typedef std::vector< ResStep > ResSteps; 
typedef std::set < ::Minisat::Lit> LitSet; 

class ResChain {
private:
  ClauseId       d_start;
  ResSteps       d_steps;
  LitSet*        d_redundantLits;
public:
  ResChain(ClauseId start);
  void addStep(::Minisat::Lit, ClauseId, bool);
  bool redundantRemoved() { return (d_redundantLits == NULL || d_redundantLits->empty()); }
  void addRedundantLit(::Minisat::Lit lit); 
  ~ResChain();
  // accessor methods
  ClauseId  getStart()     { return d_start; }
  ResSteps& getSteps()     { return d_steps; }
  LitSet*   getRedundant() { return d_redundantLits; }
};/* class ResChain */

typedef std::hash_map < ClauseId, ::Minisat::CRef > IdClauseMap;
typedef std::hash_map < ::Minisat::CRef, ClauseId > ClauseIdMap;
typedef std::hash_map < ClauseId, ::Minisat::Lit>   IdUnitMap;
typedef std::hash_map < int, ClauseId>            UnitIdMap; //FIXME 
typedef std::hash_map < ClauseId, ResChain*>      IdResMap; 
typedef std::hash_set < ClauseId >                IdHashSet;
typedef std::vector   < ResChain* >               ResStack; 

typedef std::hash_set < int >                     VarSet; 
typedef std::set < ClauseId >                     IdSet; 
typedef std::vector < ::Minisat::Lit >              LitVector; 
typedef __gnu_cxx::hash_map<Expr, ::Minisat::Lit, ExprHashFunction >  AtomToVar; 

class SatProof; 

class ProofProxy : public ProofProxyAbstract {
private:
  SatProof* d_proof;
public:
  ProofProxy(SatProof* pf);
  void updateCRef(::Minisat::CRef oldref, ::Minisat::CRef newref);
};/* class ProofProxy */

class SatProof : public Proof {
protected:
  ::Minisat::Solver*    d_solver;
  // clauses 
  IdClauseMap         d_idClause;
  ClauseIdMap         d_clauseId;
  IdUnitMap           d_idUnit;
  UnitIdMap           d_unitId;
  IdHashSet           d_deleted;
  IdHashSet           d_inputClauses; 
  
  // resolutions 
  IdResMap            d_resChains;
  ResStack            d_resStack; 
  bool                d_checkRes;
  
  static ClauseId     d_idCounter; 
  const ClauseId      d_emptyClauseId;
  const ClauseId      d_nullId;
  // proxy class to break circular dependencies 
  ProofProxy*         d_proxy;
  
  // temporary map for updating CRefs
  ClauseIdMap         d_temp_clauseId;
  IdClauseMap         d_temp_idClause;

  // unit conflict
  ClauseId d_unitConflictId;
  bool d_storedUnitConflict;

  // atom mapping
  AtomToVar d_atomToVar;
public:  
  SatProof(::Minisat::Solver* solver, bool checkRes = false);
protected:
  void print(ClauseId id); 
  void printRes(ClauseId id);
  void printRes(ResChain* res); 
  
  bool isInputClause(ClauseId id); 
  bool isUnit(ClauseId id);
  bool isUnit(::Minisat::Lit lit); 
  bool hasResolution(ClauseId id); 
  void createLitSet(ClauseId id, LitSet& set); 
  void registerResolution(ClauseId id, ResChain* res);
  
  ClauseId      getClauseId(::Minisat::CRef clause);
  ClauseId      getClauseId(::Minisat::Lit lit); 
  ::Minisat::CRef getClauseRef(ClauseId id);
  ::Minisat::Lit  getUnit(ClauseId id);
  ClauseId      getUnitId(::Minisat::Lit lit); 
  ::Minisat::Clause& getClause(ClauseId id);
  virtual void toStream(std::ostream& out);

  bool checkResolution(ClauseId id);
  /** 
   * Constructs a resolution tree that proves lit
   * and returns the ClauseId for the unit clause lit
   * @param lit the literal we are proving
   * 
   * @return 
   */
  ClauseId resolveUnit(::Minisat::Lit lit);
  /** 
   * Does a depth first search on removed literals and adds the literals
   * to be removed in the proper order to the stack. 
   * 
   * @param lit the literal we are recursing on
   * @param removedSet the previously computed set of redundant literals
   * @param removeStack the stack of literals in reverse order of resolution
   */
  void removedDfs(::Minisat::Lit lit, LitSet* removedSet, LitVector& removeStack, LitSet& inClause, LitSet& seen);
  void removeRedundantFromRes(ResChain* res, ClauseId id);
public:
  void startResChain(::Minisat::CRef start);
  void addResolutionStep(::Minisat::Lit lit, ::Minisat::CRef clause, bool sign);
  /** 
   * Pops the current resolution of the stack and stores it
   * in the resolution map. Also registers the 'clause' parameter
   * @param clause the clause the resolution is proving 
   */
  void endResChain(::Minisat::CRef clause);
  void endResChain(::Minisat::Lit lit);
  /** 
   * Stores in the current derivation the redundant literals that were 
   * eliminated from the conflict clause during conflict clause minimization. 
   * @param lit the eliminated literal 
   */
  void storeLitRedundant(::Minisat::Lit lit);

  /// update the CRef Id maps when Minisat does memory reallocation x
  void updateCRef(::Minisat::CRef old_ref, ::Minisat::CRef new_ref);
  void finishUpdateCRef();
  
  /** 
   * Constructs the empty clause resolution from the final conflict
   * 
   * @param conflict 
   */
  void finalizeProof(::Minisat::CRef conflict);

  /// clause registration methods 
  ClauseId registerClause(const ::Minisat::CRef clause, bool isInput = false);
  ClauseId registerUnitClause(const ::Minisat::Lit lit, bool isInput = false);

  void storeUnitConflict(::Minisat::Lit lit); 
  
  /** 
   * Marks the deleted clauses as deleted. Note we may still use them in the final
   * resolution. 
   * @param clause 
   */
  void markDeleted(::Minisat::CRef clause);
  /** 
   * Constructs the resolution of ~q and resolves it with the current
   * resolution thus eliminating q from the current clause
   * @param q the literal to be resolved out
   */
  void     resolveOutUnit(::Minisat::Lit q);
  /** 
   * Constructs the resolution of the literal lit. Called when a clause
   * containing lit becomes satisfied and is removed. 
   * @param lit 
   */
  void     storeUnitResolution(::Minisat::Lit lit); 
  
  ProofProxy* getProxy() {return d_proxy; }
  /** 
   * At mapping between literal and theory-atom it represents
   * 
   * @param literal 
   * @param atom 
   */
  void storeAtom(::Minisat::Lit literal, Expr atom);
};/* class SatProof */

class LFSCSatProof: public SatProof {
private:
  VarSet             d_seenVars;
  std::ostringstream d_atomsSS;
  std::ostringstream d_varSS;
  std::ostringstream d_lemmaSS;
  std::ostringstream d_clauseSS;
  std::ostringstream d_paren; 
  IdSet              d_seenLemmas;
  IdHashSet          d_seenInput; 

  inline std::string varName(::Minisat::Lit lit);
  inline std::string clauseName(ClauseId id); 

  void collectLemmas(ClauseId id);
  void printResolution(ClauseId id);
  void printInputClause(ClauseId id);

  void printVariables();
  void printClauses();
  void flush(std::ostream& out);
  void printAtoms(); 
public:
  LFSCSatProof(::Minisat::Solver* solver, bool checkRes = false):
    SatProof(solver, checkRes),
    d_seenVars(),
    d_atomsSS(), 
    d_varSS(),
    d_lemmaSS(),
    d_paren(),
    d_seenLemmas(),
    d_seenInput()
  {} 
  virtual void toStream(std::ostream& out);  
};/* class LFSCSatProof */

}/* CVC4 namespace */

#endif /* __CVC4__SAT__PROOF_H */

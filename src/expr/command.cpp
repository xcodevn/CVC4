/*********************                                                        */
/*! \file command.cpp
 ** \verbatim
 ** Original author: Morgan Deters
 ** Major contributors: Francois Bobot
 ** Minor contributors (to current version): Kshitij Bansal, Dejan Jovanovic, Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2013  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Implementation of command objects.
 **
 ** Implementation of command objects.
 **/

#include <iostream>
#include <vector>
#include <utility>
#include <iterator>
#include <sstream>
#include <exception>

#include "expr/command.h"
#include "smt/smt_engine.h"
#include "options/options.h"
#include "smt/options.h"
#include "smt/smt_engine_scope.h"
#include "util/output.h"
#include "util/dump.h"
#include "util/sexpr.h"
#include "util/util_model.h"
#include "expr/node.h"
#include "printer/printer.h"

using namespace std;

namespace CVC4 {

const int CommandPrintSuccess::s_iosIndex = std::ios_base::xalloc();
const CommandSuccess* CommandSuccess::s_instance = new CommandSuccess();

std::ostream& operator<<(std::ostream& out, const Command& c) throw() {
  c.toStream(out,
             Node::setdepth::getDepth(out),
             Node::printtypes::getPrintTypes(out),
             Node::dag::getDag(out),
             Node::setlanguage::getLanguage(out));
  return out;
}

ostream& operator<<(ostream& out, const Command* c) throw() {
  if(c == NULL) {
    out << "null";
  } else {
    out << *c;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const CommandStatus& s) throw() {
  s.toStream(out, Node::setlanguage::getLanguage(out));
  return out;
}

ostream& operator<<(ostream& out, const CommandStatus* s) throw() {
  if(s == NULL) {
    out << "null";
  } else {
    out << *s;
  }
  return out;
}

/* class Command */

Command::Command() throw() : d_commandStatus(NULL), d_muted(false) {
}

Command::Command(const Command& cmd) {
  d_commandStatus = (cmd.d_commandStatus == NULL) ? NULL : &cmd.d_commandStatus->clone();
  d_muted = cmd.d_muted;
}

Command::~Command() throw() {
  if(d_commandStatus != NULL && d_commandStatus != CommandSuccess::instance()) {
    delete d_commandStatus;
  }
}

bool Command::ok() const throw() {
  // either we haven't run the command yet, or it ran successfully
  return d_commandStatus == NULL || dynamic_cast<const CommandSuccess*>(d_commandStatus) != NULL;
}

bool Command::fail() const throw() {
  return d_commandStatus != NULL && dynamic_cast<const CommandFailure*>(d_commandStatus) != NULL;
}

void Command::invoke(SmtEngine* smtEngine, std::ostream& out) throw() {
  invoke(smtEngine);
  if(!(isMuted() && ok())) {
    printResult(out, smtEngine->getOption("command-verbosity:" + getCommandName()).getIntegerValue().toUnsignedInt());
  }
}

std::string Command::toString() const throw() {
  std::stringstream ss;
  toStream(ss);
  return ss.str();
}

void Command::toStream(std::ostream& out, int toDepth, bool types, size_t dag,
                       OutputLanguage language) const throw() {
  Printer::getPrinter(language)->toStream(out, this, toDepth, types, dag);
}

void CommandStatus::toStream(std::ostream& out, OutputLanguage language) const throw() {
  Printer::getPrinter(language)->toStream(out, this);
}

void Command::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(d_commandStatus != NULL) {
    if((!ok() && verbosity >= 1) || verbosity >= 2) {
      out << *d_commandStatus;
    }
  }
}

/* class EmptyCommand */

EmptyCommand::EmptyCommand(std::string name) throw() :
  d_name(name) {
}

std::string EmptyCommand::getName() const throw() {
  return d_name;
}

void EmptyCommand::invoke(SmtEngine* smtEngine) throw() {
  /* empty commands have no implementation */
  d_commandStatus = CommandSuccess::instance();
}

Command* EmptyCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new EmptyCommand(d_name);
}

Command* EmptyCommand::clone() const {
  return new EmptyCommand(d_name);
}

std::string EmptyCommand::getCommandName() const throw() {
  return "empty";
}

/* class EchoCommand */

EchoCommand::EchoCommand(std::string output) throw() :
  d_output(output) {
}

std::string EchoCommand::getOutput() const throw() {
  return d_output;
}

void EchoCommand::invoke(SmtEngine* smtEngine) throw() {
  /* we don't have an output stream here, nothing to do */
  d_commandStatus = CommandSuccess::instance();
}

void EchoCommand::invoke(SmtEngine* smtEngine, std::ostream& out) throw() {
  out << d_output << std::endl;
  d_commandStatus = CommandSuccess::instance();
  printResult(out, smtEngine->getOption("command-verbosity:" + getCommandName()).getIntegerValue().toUnsignedInt());
}

Command* EchoCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new EchoCommand(d_output);
}

Command* EchoCommand::clone() const {
  return new EchoCommand(d_output);
}

std::string EchoCommand::getCommandName() const throw() {
  return "echo";
}

/* class AssertCommand */

AssertCommand::AssertCommand(const Expr& e) throw() :
  d_expr(e) {
}

Expr AssertCommand::getExpr() const throw() {
  return d_expr;
}

void AssertCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    smtEngine->assertFormula(d_expr);
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Command* AssertCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new AssertCommand(d_expr.exportTo(exprManager, variableMap));
}

Command* AssertCommand::clone() const {
  return new AssertCommand(d_expr);
}

std::string AssertCommand::getCommandName() const throw() {
  return "assert";
}


/* class PushCommand */

void PushCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    smtEngine->push();
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Command* PushCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new PushCommand();
}

Command* PushCommand::clone() const {
  return new PushCommand();
}

std::string PushCommand::getCommandName() const throw() {
  return "push";
}

/* class PopCommand */

void PopCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    smtEngine->pop();
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Command* PopCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new PopCommand();
}

Command* PopCommand::clone() const {
  return new PopCommand();
}

std::string PopCommand::getCommandName() const throw() {
  return "pop";
}

/* class CheckSatCommand */

CheckSatCommand::CheckSatCommand() throw() :
  d_expr() {
}

CheckSatCommand::CheckSatCommand(const Expr& expr) throw() :
  d_expr(expr) {
}

Expr CheckSatCommand::getExpr() const throw() {
  return d_expr;
}

void CheckSatCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    d_result = smtEngine->checkSat(d_expr);
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Result CheckSatCommand::getResult() const throw() {
  return d_result;
}

void CheckSatCommand::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(! ok()) {
    this->Command::printResult(out, verbosity);
  } else {
    out << d_result << endl;
  }
}

Command* CheckSatCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  CheckSatCommand* c = new CheckSatCommand(d_expr.exportTo(exprManager, variableMap));
  c->d_result = d_result;
  return c;
}

Command* CheckSatCommand::clone() const {
  CheckSatCommand* c = new CheckSatCommand(d_expr);
  c->d_result = d_result;
  return c;
}

std::string CheckSatCommand::getCommandName() const throw() {
  return "check-sat";
}

/* class QueryCommand */

QueryCommand::QueryCommand(const Expr& e) throw() :
  d_expr(e) {
}

Expr QueryCommand::getExpr() const throw() {
  return d_expr;
}

void QueryCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    d_result = smtEngine->query(d_expr);
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Result QueryCommand::getResult() const throw() {
  return d_result;
}

void QueryCommand::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(! ok()) {
    this->Command::printResult(out, verbosity);
  } else {
    out << d_result << endl;
  }
}

Command* QueryCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  QueryCommand* c = new QueryCommand(d_expr.exportTo(exprManager, variableMap));
  c->d_result = d_result;
  return c;
}

Command* QueryCommand::clone() const {
  QueryCommand* c = new QueryCommand(d_expr);
  c->d_result = d_result;
  return c;
}

std::string QueryCommand::getCommandName() const throw() {
  return "query";
}

/* class QuitCommand */

QuitCommand::QuitCommand() throw() {
}

void QuitCommand::invoke(SmtEngine* smtEngine) throw() {
  Dump("benchmark") << *this;
  d_commandStatus = CommandSuccess::instance();
}

Command* QuitCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new QuitCommand();
}

Command* QuitCommand::clone() const {
  return new QuitCommand();
}

std::string QuitCommand::getCommandName() const throw() {
  return "exit";
}

/* class CommentCommand */

CommentCommand::CommentCommand(std::string comment) throw() : d_comment(comment) {
}

std::string CommentCommand::getComment() const throw() {
  return d_comment;
}

void CommentCommand::invoke(SmtEngine* smtEngine) throw() {
  Dump("benchmark") << *this;
  d_commandStatus = CommandSuccess::instance();
}

Command* CommentCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new CommentCommand(d_comment);
}

Command* CommentCommand::clone() const {
  return new CommentCommand(d_comment);
}

std::string CommentCommand::getCommandName() const throw() {
  return "comment";
}

/* class CommandSequence */

CommandSequence::CommandSequence() throw() :
  d_index(0) {
}

CommandSequence::~CommandSequence() throw() {
  for(unsigned i = d_index; i < d_commandSequence.size(); ++i) {
    delete d_commandSequence[i];
  }
}

void CommandSequence::addCommand(Command* cmd) throw() {
  d_commandSequence.push_back(cmd);
}

void CommandSequence::clear() throw() {
  d_commandSequence.clear();
}

void CommandSequence::invoke(SmtEngine* smtEngine) throw() {
  for(; d_index < d_commandSequence.size(); ++d_index) {
    d_commandSequence[d_index]->invoke(smtEngine);
    if(! d_commandSequence[d_index]->ok()) {
      // abort execution
      d_commandStatus = d_commandSequence[d_index]->getCommandStatus();
      return;
    }
    delete d_commandSequence[d_index];
  }

  AlwaysAssert(d_commandStatus == NULL);
  d_commandStatus = CommandSuccess::instance();
}

void CommandSequence::invoke(SmtEngine* smtEngine, std::ostream& out) throw() {
  for(; d_index < d_commandSequence.size(); ++d_index) {
    d_commandSequence[d_index]->invoke(smtEngine, out);
    if(! d_commandSequence[d_index]->ok()) {
      // abort execution
      d_commandStatus = d_commandSequence[d_index]->getCommandStatus();
      return;
    }
    delete d_commandSequence[d_index];
  }

  AlwaysAssert(d_commandStatus == NULL);
  d_commandStatus = CommandSuccess::instance();
}

Command* CommandSequence::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  CommandSequence* seq = new CommandSequence();
  for(iterator i = begin(); i != end(); ++i) {
    Command* cmd_to_export = *i;
    Command* cmd = cmd_to_export->exportTo(exprManager, variableMap);
    seq->addCommand(cmd);
    Debug("export") << "[export] so far coverted: " << seq << endl;
  }
  seq->d_index = d_index;
  return seq;
}

Command* CommandSequence::clone() const {
  CommandSequence* seq = new CommandSequence();
  for(const_iterator i = begin(); i != end(); ++i) {
    seq->addCommand((*i)->clone());
  }
  seq->d_index = d_index;
  return seq;
}

CommandSequence::const_iterator CommandSequence::begin() const throw() {
  return d_commandSequence.begin();
}

CommandSequence::const_iterator CommandSequence::end() const throw() {
  return d_commandSequence.end();
}

CommandSequence::iterator CommandSequence::begin() throw() {
  return d_commandSequence.begin();
}

CommandSequence::iterator CommandSequence::end() throw() {
  return d_commandSequence.end();
}

std::string CommandSequence::getCommandName() const throw() {
  return "sequence";
}

/* class DeclarationSequenceCommand */

/* class DeclarationDefinitionCommand */

DeclarationDefinitionCommand::DeclarationDefinitionCommand(const std::string& id) throw() :
  d_symbol(id) {
}

std::string DeclarationDefinitionCommand::getSymbol() const throw() {
  return d_symbol;
}

/* class DeclareFunctionCommand */

DeclareFunctionCommand::DeclareFunctionCommand(const std::string& id, Expr func, Type t) throw() :
  DeclarationDefinitionCommand(id),
  d_func(func),
  d_type(t) {
}

Expr DeclareFunctionCommand::getFunction() const throw() {
  return d_func;
}

Type DeclareFunctionCommand::getType() const throw() {
  return d_type;
}

void DeclareFunctionCommand::invoke(SmtEngine* smtEngine) throw() {
  d_commandStatus = CommandSuccess::instance();
}

Command* DeclareFunctionCommand::exportTo(ExprManager* exprManager,
                                          ExprManagerMapCollection& variableMap) {
  return new DeclareFunctionCommand(d_symbol, d_func.exportTo(exprManager, variableMap),
                                    d_type.exportTo(exprManager, variableMap));
}

Command* DeclareFunctionCommand::clone() const {
  return new DeclareFunctionCommand(d_symbol, d_func, d_type);
}

std::string DeclareFunctionCommand::getCommandName() const throw() {
  return "declare-fun";
}

/* class DeclareTypeCommand */

DeclareTypeCommand::DeclareTypeCommand(const std::string& id, size_t arity, Type t) throw() :
  DeclarationDefinitionCommand(id),
  d_arity(arity),
  d_type(t) {
}

size_t DeclareTypeCommand::getArity() const throw() {
  return d_arity;
}

Type DeclareTypeCommand::getType() const throw() {
  return d_type;
}

void DeclareTypeCommand::invoke(SmtEngine* smtEngine) throw() {
  d_commandStatus = CommandSuccess::instance();
}

Command* DeclareTypeCommand::exportTo(ExprManager* exprManager,
                                      ExprManagerMapCollection& variableMap) {
  return new DeclareTypeCommand(d_symbol, d_arity,
                                d_type.exportTo(exprManager, variableMap));
}

Command* DeclareTypeCommand::clone() const {
  return new DeclareTypeCommand(d_symbol, d_arity, d_type);
}

std::string DeclareTypeCommand::getCommandName() const throw() {
  return "declare-sort";
}

/* class DefineTypeCommand */

DefineTypeCommand::DefineTypeCommand(const std::string& id,
                                     Type t) throw() :
  DeclarationDefinitionCommand(id),
  d_params(),
  d_type(t) {
}

DefineTypeCommand::DefineTypeCommand(const std::string& id,
                                     const std::vector<Type>& params,
                                     Type t) throw() :
  DeclarationDefinitionCommand(id),
  d_params(params),
  d_type(t) {
}

const std::vector<Type>& DefineTypeCommand::getParameters() const throw() {
  return d_params;
}

Type DefineTypeCommand::getType() const throw() {
  return d_type;
}

void DefineTypeCommand::invoke(SmtEngine* smtEngine) throw() {
  d_commandStatus = CommandSuccess::instance();
}

Command* DefineTypeCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  vector<Type> params;
  transform(d_params.begin(), d_params.end(), back_inserter(params),
            ExportTransformer(exprManager, variableMap));
  Type type = d_type.exportTo(exprManager, variableMap);
  return new DefineTypeCommand(d_symbol, params, type);
}

Command* DefineTypeCommand::clone() const {
  return new DefineTypeCommand(d_symbol, d_params, d_type);
}

std::string DefineTypeCommand::getCommandName() const throw() {
  return "define-sort";
}

/* class DefineFunctionCommand */

DefineFunctionCommand::DefineFunctionCommand(const std::string& id,
                                             Expr func,
                                             Expr formula) throw() :
  DeclarationDefinitionCommand(id),
  d_func(func),
  d_formals(),
  d_formula(formula) {
}

DefineFunctionCommand::DefineFunctionCommand(const std::string& id,
                                             Expr func,
                                             const std::vector<Expr>& formals,
                                             Expr formula) throw() :
  DeclarationDefinitionCommand(id),
  d_func(func),
  d_formals(formals),
  d_formula(formula) {
}

Expr DefineFunctionCommand::getFunction() const throw() {
  return d_func;
}

const std::vector<Expr>& DefineFunctionCommand::getFormals() const throw() {
  return d_formals;
}

Expr DefineFunctionCommand::getFormula() const throw() {
  return d_formula;
}

void DefineFunctionCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    if(!d_func.isNull()) {
      smtEngine->defineFunction(d_func, d_formals, d_formula);
    }
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Command* DefineFunctionCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  Expr func = d_func.exportTo(exprManager, variableMap, /* flags = */ ExprManager::VAR_FLAG_DEFINED);
  vector<Expr> formals;
  transform(d_formals.begin(), d_formals.end(), back_inserter(formals),
            ExportTransformer(exprManager, variableMap));
  Expr formula = d_formula.exportTo(exprManager, variableMap);
  return new DefineFunctionCommand(d_symbol, func, formals, formula);
}

Command* DefineFunctionCommand::clone() const {
  return new DefineFunctionCommand(d_symbol, d_func, d_formals, d_formula);
}

std::string DefineFunctionCommand::getCommandName() const throw() {
  return "define-fun";
}

/* class DefineNamedFunctionCommand */

DefineNamedFunctionCommand::DefineNamedFunctionCommand(const std::string& id,
                                                       Expr func,
                                                       const std::vector<Expr>& formals,
                                                       Expr formula) throw() :
  DefineFunctionCommand(id, func, formals, formula) {
}

void DefineNamedFunctionCommand::invoke(SmtEngine* smtEngine) throw() {
  this->DefineFunctionCommand::invoke(smtEngine);
  if(!d_func.isNull() && d_func.getType().isBoolean()) {
    smtEngine->addToAssignment(d_func.getExprManager()->mkExpr(kind::APPLY, d_func));
  }
  d_commandStatus = CommandSuccess::instance();
}

Command* DefineNamedFunctionCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  Expr func = d_func.exportTo(exprManager, variableMap);
  vector<Expr> formals;
  transform(d_formals.begin(), d_formals.end(), back_inserter(formals),
            ExportTransformer(exprManager, variableMap));
  Expr formula = d_formula.exportTo(exprManager, variableMap);
  return new DefineNamedFunctionCommand(d_symbol, func, formals, formula);
}

Command* DefineNamedFunctionCommand::clone() const {
  return new DefineNamedFunctionCommand(d_symbol, d_func, d_formals, d_formula);
}

/* class SetUserAttribute */

SetUserAttributeCommand::SetUserAttributeCommand( const std::string& attr, Expr expr ) throw() :
  d_attr( attr ), d_expr( expr ){
}
/*
SetUserAttributeCommand::SetUserAttributeCommand( const std::string& id, Expr expr,
                                                  std::vector<Expr>& values ) throw() :
  d_id( id ), d_expr( expr ){
  d_expr_values.insert( d_expr_values.begin(), values.begin(), values.end() );
}

SetUserAttributeCommand::SetUserAttributeCommand( const std::string& id, Expr expr,
                                                  std::string& value ) throw() :
  d_id( id ), d_expr( expr ), d_str_value( value ){
}
*/
void SetUserAttributeCommand::invoke(SmtEngine* smtEngine) throw(){
  try {
    if(!d_expr.isNull()) {
      smtEngine->setUserAttribute( d_attr, d_expr );
    }
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Command* SetUserAttributeCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap){
  Expr expr = d_expr.exportTo(exprManager, variableMap);
  return new SetUserAttributeCommand( d_attr, expr );
}

Command* SetUserAttributeCommand::clone() const{
  return new SetUserAttributeCommand( d_attr, d_expr );
}

std::string SetUserAttributeCommand::getCommandName() const throw() {
  return "set-user-attribute";
}

/* class SimplifyCommand */

SimplifyCommand::SimplifyCommand(Expr term) throw() :
  d_term(term) {
}

Expr SimplifyCommand::getTerm() const throw() {
  return d_term;
}

void SimplifyCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    d_result = smtEngine->simplify(d_term);
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Expr SimplifyCommand::getResult() const throw() {
  return d_result;
}

void SimplifyCommand::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(! ok()) {
    this->Command::printResult(out, verbosity);
  } else {
    out << d_result << endl;
  }
}

Command* SimplifyCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  SimplifyCommand* c = new SimplifyCommand(d_term.exportTo(exprManager, variableMap));
  c->d_result = d_result.exportTo(exprManager, variableMap);
  return c;
}

Command* SimplifyCommand::clone() const {
  SimplifyCommand* c = new SimplifyCommand(d_term);
  c->d_result = d_result;
  return c;
}

std::string SimplifyCommand::getCommandName() const throw() {
  return "simplify";
}

/* class ExpandDefinitionsCommand */

ExpandDefinitionsCommand::ExpandDefinitionsCommand(Expr term) throw() :
  d_term(term) {
}

Expr ExpandDefinitionsCommand::getTerm() const throw() {
  return d_term;
}

void ExpandDefinitionsCommand::invoke(SmtEngine* smtEngine) throw() {
  d_result = smtEngine->expandDefinitions(d_term);
  d_commandStatus = CommandSuccess::instance();
}

Expr ExpandDefinitionsCommand::getResult() const throw() {
  return d_result;
}

void ExpandDefinitionsCommand::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(! ok()) {
    this->Command::printResult(out, verbosity);
  } else {
    out << d_result << endl;
  }
}

Command* ExpandDefinitionsCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  ExpandDefinitionsCommand* c = new ExpandDefinitionsCommand(d_term.exportTo(exprManager, variableMap));
  c->d_result = d_result.exportTo(exprManager, variableMap);
  return c;
}

Command* ExpandDefinitionsCommand::clone() const {
  ExpandDefinitionsCommand* c = new ExpandDefinitionsCommand(d_term);
  c->d_result = d_result;
  return c;
}

std::string ExpandDefinitionsCommand::getCommandName() const throw() {
  return "expand-definitions";
}

/* class GetValueCommand */

GetValueCommand::GetValueCommand(Expr term) throw() :
  d_terms() {
  d_terms.push_back(term);
}

GetValueCommand::GetValueCommand(const std::vector<Expr>& terms) throw() :
  d_terms(terms) {
  CheckArgument(terms.size() >= 1, terms, "cannot get-value of an empty set of terms");
}

const std::vector<Expr>& GetValueCommand::getTerms() const throw() {
  return d_terms;
}

void GetValueCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    vector<Expr> result;
    ExprManager* em = smtEngine->getExprManager();
    NodeManager* nm = NodeManager::fromExprManager(em);
    for(std::vector<Expr>::const_iterator i = d_terms.begin(); i != d_terms.end(); ++i) {
      Assert(nm == NodeManager::fromExprManager((*i).getExprManager()));
      smt::SmtScope scope(smtEngine);
      Node request = Node::fromExpr(options::expandDefinitions() ? smtEngine->expandDefinitions(*i) : *i);
      Node value = Node::fromExpr(smtEngine->getValue(*i));
      result.push_back(nm->mkNode(kind::SEXPR, request, value).toExpr());
    }
    d_result = em->mkExpr(kind::SEXPR, result);
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Expr GetValueCommand::getResult() const throw() {
  return d_result;
}

void GetValueCommand::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(! ok()) {
    this->Command::printResult(out, verbosity);
  } else {
    Expr::dag::Scope scope(out, false);
    out << d_result << endl;
  }
}

Command* GetValueCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  vector<Expr> exportedTerms;
  for(std::vector<Expr>::const_iterator i = d_terms.begin(); i != d_terms.end(); ++i) {
    exportedTerms.push_back((*i).exportTo(exprManager, variableMap));
  }
  GetValueCommand* c = new GetValueCommand(exportedTerms);
  c->d_result = d_result.exportTo(exprManager, variableMap);
  return c;
}

Command* GetValueCommand::clone() const {
  GetValueCommand* c = new GetValueCommand(d_terms);
  c->d_result = d_result;
  return c;
}

std::string GetValueCommand::getCommandName() const throw() {
  return "get-value";
}

/* class GetAssignmentCommand */

GetAssignmentCommand::GetAssignmentCommand() throw() {
}

void GetAssignmentCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    d_result = smtEngine->getAssignment();
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

SExpr GetAssignmentCommand::getResult() const throw() {
  return d_result;
}

void GetAssignmentCommand::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(! ok()) {
    this->Command::printResult(out, verbosity);
  } else {
    out << d_result << endl;
  }
}

Command* GetAssignmentCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetAssignmentCommand* c = new GetAssignmentCommand();
  c->d_result = d_result;
  return c;
}

Command* GetAssignmentCommand::clone() const {
  GetAssignmentCommand* c = new GetAssignmentCommand();
  c->d_result = d_result;
  return c;
}

std::string GetAssignmentCommand::getCommandName() const throw() {
  return "get-assignment";
}

/* class GetModelCommand */

GetModelCommand::GetModelCommand() throw() {
}

void GetModelCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    d_result = smtEngine->getModel();
    d_smtEngine = smtEngine;
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

/* Model is private to the library -- for now
Model* GetModelCommand::getResult() const throw() {
  return d_result;
}
*/

void GetModelCommand::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(! ok()) {
    this->Command::printResult(out, verbosity);
  } else {
    out << *d_result;
  }
}

Command* GetModelCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetModelCommand* c = new GetModelCommand();
  c->d_result = d_result;
  c->d_smtEngine = d_smtEngine;
  return c;
}

Command* GetModelCommand::clone() const {
  GetModelCommand* c = new GetModelCommand();
  c->d_result = d_result;
  c->d_smtEngine = d_smtEngine;
  return c;
}

std::string GetModelCommand::getCommandName() const throw() {
  return "get-model";
}

/* class GetProofCommand */

GetProofCommand::GetProofCommand() throw() {
}

void GetProofCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    d_result = smtEngine->getProof();
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Proof* GetProofCommand::getResult() const throw() {
  return d_result;
}

void GetProofCommand::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(! ok()) {
    this->Command::printResult(out, verbosity);
  } else {
    d_result->toStream(out);
  }
}

Command* GetProofCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetProofCommand* c = new GetProofCommand();
  c->d_result = d_result;
  return c;
}

Command* GetProofCommand::clone() const {
  GetProofCommand* c = new GetProofCommand();
  c->d_result = d_result;
  return c;
}

std::string GetProofCommand::getCommandName() const throw() {
  return "get-proof";
}

/* class GetUnsatCoreCommand */

GetUnsatCoreCommand::GetUnsatCoreCommand() throw() {
}

void GetUnsatCoreCommand::invoke(SmtEngine* smtEngine) throw() {
  /*
  try {
    d_result = smtEngine->getUnsatCore();
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
  */
  d_commandStatus = new CommandUnsupported();
}

void GetUnsatCoreCommand::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(! ok()) {
    this->Command::printResult(out, verbosity);
  } else {
    //do nothing -- unsat cores not yet supported
    // d_result->toStream(out);
  }
}

Command* GetUnsatCoreCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetUnsatCoreCommand* c = new GetUnsatCoreCommand();
  //c->d_result = d_result;
  return c;
}

Command* GetUnsatCoreCommand::clone() const {
  GetUnsatCoreCommand* c = new GetUnsatCoreCommand();
  //c->d_result = d_result;
  return c;
}

std::string GetUnsatCoreCommand::getCommandName() const throw() {
  return "get-unsat-core";
}

/* class GetAssertionsCommand */

GetAssertionsCommand::GetAssertionsCommand() throw() {
}

void GetAssertionsCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    stringstream ss;
    const vector<Expr> v = smtEngine->getAssertions();
    ss << "(\n";
    copy( v.begin(), v.end(), ostream_iterator<Expr>(ss, "\n") );
    ss << ")\n";
    d_result = ss.str();
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

std::string GetAssertionsCommand::getResult() const throw() {
  return d_result;
}

void GetAssertionsCommand::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(! ok()) {
    this->Command::printResult(out, verbosity);
  } else {
    out << d_result;
  }
}

Command* GetAssertionsCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetAssertionsCommand* c = new GetAssertionsCommand();
  c->d_result = d_result;
  return c;
}

Command* GetAssertionsCommand::clone() const {
  GetAssertionsCommand* c = new GetAssertionsCommand();
  c->d_result = d_result;
  return c;
}

std::string GetAssertionsCommand::getCommandName() const throw() {
  return "get-assertions";
}

/* class SetBenchmarkStatusCommand */

SetBenchmarkStatusCommand::SetBenchmarkStatusCommand(BenchmarkStatus status) throw() :
  d_status(status) {
}

BenchmarkStatus SetBenchmarkStatusCommand::getStatus() const throw() {
  return d_status;
}

void SetBenchmarkStatusCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    stringstream ss;
    ss << d_status;
    SExpr status = ss.str();
    smtEngine->setInfo("status", status);
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Command* SetBenchmarkStatusCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new SetBenchmarkStatusCommand(d_status);
}

Command* SetBenchmarkStatusCommand::clone() const {
  return new SetBenchmarkStatusCommand(d_status);
}

std::string SetBenchmarkStatusCommand::getCommandName() const throw() {
  return "set-info";
}

/* class SetBenchmarkLogicCommand */

SetBenchmarkLogicCommand::SetBenchmarkLogicCommand(std::string logic) throw() :
  d_logic(logic) {
}

std::string SetBenchmarkLogicCommand::getLogic() const throw() {
  return d_logic;
}

void SetBenchmarkLogicCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    smtEngine->setLogic(d_logic);
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Command* SetBenchmarkLogicCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new SetBenchmarkLogicCommand(d_logic);
}

Command* SetBenchmarkLogicCommand::clone() const {
  return new SetBenchmarkLogicCommand(d_logic);
}

std::string SetBenchmarkLogicCommand::getCommandName() const throw() {
  return "set-logic";
}

/* class SetInfoCommand */

SetInfoCommand::SetInfoCommand(std::string flag, const SExpr& sexpr) throw() :
  d_flag(flag),
  d_sexpr(sexpr) {
}

std::string SetInfoCommand::getFlag() const throw() {
  return d_flag;
}

SExpr SetInfoCommand::getSExpr() const throw() {
  return d_sexpr;
}

void SetInfoCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    smtEngine->setInfo(d_flag, d_sexpr);
    d_commandStatus = CommandSuccess::instance();
  } catch(UnrecognizedOptionException&) {
    // As per SMT-LIB spec, silently accept unknown set-info keys
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Command* SetInfoCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new SetInfoCommand(d_flag, d_sexpr);
}

Command* SetInfoCommand::clone() const {
  return new SetInfoCommand(d_flag, d_sexpr);
}

std::string SetInfoCommand::getCommandName() const throw() {
  return "set-info";
}

/* class GetInfoCommand */

GetInfoCommand::GetInfoCommand(std::string flag) throw() :
  d_flag(flag) {
}

std::string GetInfoCommand::getFlag() const throw() {
  return d_flag;
}

void GetInfoCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    vector<SExpr> v;
    v.push_back(SExpr(SExpr::Keyword(string(":") + d_flag)));
    v.push_back(smtEngine->getInfo(d_flag));
    stringstream ss;
    ss << SExpr(v);
    d_result = ss.str();
    d_commandStatus = CommandSuccess::instance();
  } catch(UnrecognizedOptionException&) {
    d_commandStatus = new CommandUnsupported();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

std::string GetInfoCommand::getResult() const throw() {
  return d_result;
}

void GetInfoCommand::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(! ok()) {
    this->Command::printResult(out, verbosity);
  } else if(d_result != "") {
    out << d_result << endl;
  }
}

Command* GetInfoCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetInfoCommand* c = new GetInfoCommand(d_flag);
  c->d_result = d_result;
  return c;
}

Command* GetInfoCommand::clone() const {
  GetInfoCommand* c = new GetInfoCommand(d_flag);
  c->d_result = d_result;
  return c;
}

std::string GetInfoCommand::getCommandName() const throw() {
  return "get-info";
}

/* class SetOptionCommand */

SetOptionCommand::SetOptionCommand(std::string flag, const SExpr& sexpr) throw() :
  d_flag(flag),
  d_sexpr(sexpr) {
}

std::string SetOptionCommand::getFlag() const throw() {
  return d_flag;
}

SExpr SetOptionCommand::getSExpr() const throw() {
  return d_sexpr;
}

void SetOptionCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    smtEngine->setOption(d_flag, d_sexpr);
    d_commandStatus = CommandSuccess::instance();
  } catch(UnrecognizedOptionException&) {
    d_commandStatus = new CommandUnsupported();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Command* SetOptionCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  return new SetOptionCommand(d_flag, d_sexpr);
}

Command* SetOptionCommand::clone() const {
  return new SetOptionCommand(d_flag, d_sexpr);
}

std::string SetOptionCommand::getCommandName() const throw() {
  return "set-option";
}

/* class GetOptionCommand */

GetOptionCommand::GetOptionCommand(std::string flag) throw() :
  d_flag(flag) {
}

std::string GetOptionCommand::getFlag() const throw() {
  return d_flag;
}

void GetOptionCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    SExpr res = smtEngine->getOption(d_flag);
    stringstream ss;
    ss << res;
    d_result = ss.str();
    d_commandStatus = CommandSuccess::instance();
  } catch(UnrecognizedOptionException&) {
    d_commandStatus = new CommandUnsupported();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

std::string GetOptionCommand::getResult() const throw() {
  return d_result;
}

void GetOptionCommand::printResult(std::ostream& out, uint32_t verbosity) const throw() {
  if(! ok()) {
    this->Command::printResult(out, verbosity);
  } else if(d_result != "") {
    out << d_result << endl;
  }
}

Command* GetOptionCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  GetOptionCommand* c = new GetOptionCommand(d_flag);
  c->d_result = d_result;
  return c;
}

Command* GetOptionCommand::clone() const {
  GetOptionCommand* c = new GetOptionCommand(d_flag);
  c->d_result = d_result;
  return c;
}

std::string GetOptionCommand::getCommandName() const throw() {
  return "get-option";
}

/* class DatatypeDeclarationCommand */

DatatypeDeclarationCommand::DatatypeDeclarationCommand(const DatatypeType& datatype) throw() :
  d_datatypes() {
  d_datatypes.push_back(datatype);
}

DatatypeDeclarationCommand::DatatypeDeclarationCommand(const std::vector<DatatypeType>& datatypes) throw() :
  d_datatypes(datatypes) {
}

const std::vector<DatatypeType>&
DatatypeDeclarationCommand::getDatatypes() const throw() {
  return d_datatypes;
}

void DatatypeDeclarationCommand::invoke(SmtEngine* smtEngine) throw() {
  d_commandStatus = CommandSuccess::instance();
}

Command* DatatypeDeclarationCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  throw ExportUnsupportedException
          ("export of DatatypeDeclarationCommand unsupported");
}

Command* DatatypeDeclarationCommand::clone() const {
  return new DatatypeDeclarationCommand(d_datatypes);
}

std::string DatatypeDeclarationCommand::getCommandName() const throw() {
  return "declare-datatypes";
}

/* class RewriteRuleCommand */

RewriteRuleCommand::RewriteRuleCommand(const std::vector<Expr>& vars,
                                       const std::vector<Expr>& guards,
                                       Expr head, Expr body,
                                       const Triggers& triggers) throw() :
  d_vars(vars), d_guards(guards), d_head(head), d_body(body), d_triggers(triggers) {
}

RewriteRuleCommand::RewriteRuleCommand(const std::vector<Expr>& vars,
                                       Expr head, Expr body) throw() :
  d_vars(vars), d_head(head), d_body(body) {
}

const std::vector<Expr>& RewriteRuleCommand::getVars() const throw() {
  return d_vars;
}

const std::vector<Expr>& RewriteRuleCommand::getGuards() const throw() {
  return d_guards;
}

Expr RewriteRuleCommand::getHead() const throw() {
  return d_head;
}

Expr RewriteRuleCommand::getBody() const throw() {
  return d_body;
}

const RewriteRuleCommand::Triggers& RewriteRuleCommand::getTriggers() const throw() {
  return d_triggers;
}

void RewriteRuleCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    ExprManager* em = smtEngine->getExprManager();
    /** build vars list */
    Expr vars = em->mkExpr(kind::BOUND_VAR_LIST, d_vars);
    /** build guards list */
    Expr guards;
    if(d_guards.size() == 0) guards = em->mkConst<bool>(true);
    else if(d_guards.size() == 1) guards = d_guards[0];
    else guards = em->mkExpr(kind::AND,d_guards);
    /** build expression */
    Expr expr;
    if( d_triggers.empty() ){
      expr = em->mkExpr(kind::RR_REWRITE,vars,guards,d_head,d_body);
    } else {
      /** build triggers list */
      std::vector<Expr> vtriggers;
      vtriggers.reserve(d_triggers.size());
      for(Triggers::const_iterator i = d_triggers.begin(),
            end = d_triggers.end(); i != end; ++i){
        vtriggers.push_back(em->mkExpr(kind::INST_PATTERN,*i));
      }
      Expr triggers = em->mkExpr(kind::INST_PATTERN_LIST,vtriggers);
      expr = em->mkExpr(kind::RR_REWRITE,vars,guards,d_head,d_body,triggers);
    }
    smtEngine->assertFormula(expr);
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Command* RewriteRuleCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  /** Convert variables */
  VExpr vars; vars.reserve(d_vars.size());
  for(VExpr::iterator i = d_vars.begin(), end = d_vars.end();
      i == end; ++i){
    vars.push_back(i->exportTo(exprManager, variableMap));
  };
  /** Convert guards */
  VExpr guards; guards.reserve(d_guards.size());
  for(VExpr::iterator i = d_guards.begin(), end = d_guards.end();
      i == end; ++i){
    guards.push_back(i->exportTo(exprManager, variableMap));
  };
  /** Convert triggers */
  Triggers triggers; triggers.resize(d_triggers.size());
  for(size_t i = 0, end = d_triggers.size();
      i < end; ++i){
    triggers[i].reserve(d_triggers[i].size());
    for(VExpr::iterator j = d_triggers[i].begin(), jend = d_triggers[i].end();
        j == jend; ++i){
      triggers[i].push_back(j->exportTo(exprManager, variableMap));
    };
  };
  /** Convert head and body */
  Expr head = d_head.exportTo(exprManager, variableMap);
  Expr body = d_body.exportTo(exprManager, variableMap);
  /** Create the converted rules */
  return new RewriteRuleCommand(vars, guards, head, body, triggers);
}

Command* RewriteRuleCommand::clone() const {
  return new RewriteRuleCommand(d_vars, d_guards, d_head, d_body, d_triggers);
}

std::string RewriteRuleCommand::getCommandName() const throw() {
  return "rewrite-rule";
}

/* class PropagateRuleCommand */

PropagateRuleCommand::PropagateRuleCommand(const std::vector<Expr>& vars,
                                           const std::vector<Expr>& guards,
                                           const std::vector<Expr>& heads,
                                           Expr body,
                                           const Triggers& triggers,
                                           bool deduction) throw() :
  d_vars(vars), d_guards(guards), d_heads(heads), d_body(body), d_triggers(triggers), d_deduction(deduction) {
}

PropagateRuleCommand::PropagateRuleCommand(const std::vector<Expr>& vars,
                                           const std::vector<Expr>& heads,
                                           Expr body,
                                           bool deduction) throw() :
  d_vars(vars), d_heads(heads), d_body(body), d_deduction(deduction) {
}

const std::vector<Expr>& PropagateRuleCommand::getVars() const throw() {
  return d_vars;
}

const std::vector<Expr>& PropagateRuleCommand::getGuards() const throw() {
  return d_guards;
}

const std::vector<Expr>& PropagateRuleCommand::getHeads() const throw() {
  return d_heads;
}

Expr PropagateRuleCommand::getBody() const throw() {
  return d_body;
}

const PropagateRuleCommand::Triggers& PropagateRuleCommand::getTriggers() const throw() {
  return d_triggers;
}

bool PropagateRuleCommand::isDeduction() const throw() {
  return d_deduction;
}

void PropagateRuleCommand::invoke(SmtEngine* smtEngine) throw() {
  try {
    ExprManager* em = smtEngine->getExprManager();
    /** build vars list */
    Expr vars = em->mkExpr(kind::BOUND_VAR_LIST, d_vars);
    /** build guards list */
    Expr guards;
    if(d_guards.size() == 0) guards = em->mkConst<bool>(true);
    else if(d_guards.size() == 1) guards = d_guards[0];
    else guards = em->mkExpr(kind::AND,d_guards);
    /** build heads list */
    Expr heads;
    if(d_heads.size() == 1) heads = d_heads[0];
    else heads = em->mkExpr(kind::AND,d_heads);
    /** build expression */
    Expr expr;
    if( d_triggers.empty() ){
      expr = em->mkExpr(kind::RR_REWRITE,vars,guards,heads,d_body);
    } else {
      /** build triggers list */
      std::vector<Expr> vtriggers;
      vtriggers.reserve(d_triggers.size());
      for(Triggers::const_iterator i = d_triggers.begin(),
            end = d_triggers.end(); i != end; ++i){
        vtriggers.push_back(em->mkExpr(kind::INST_PATTERN,*i));
      }
      Expr triggers = em->mkExpr(kind::INST_PATTERN_LIST,vtriggers);
      expr = em->mkExpr(kind::RR_REWRITE,vars,guards,heads,d_body,triggers);
    }
    smtEngine->assertFormula(expr);
    d_commandStatus = CommandSuccess::instance();
  } catch(exception& e) {
    d_commandStatus = new CommandFailure(e.what());
  }
}

Command* PropagateRuleCommand::exportTo(ExprManager* exprManager, ExprManagerMapCollection& variableMap) {
  /** Convert variables */
  VExpr vars; vars.reserve(d_vars.size());
  for(VExpr::iterator i = d_vars.begin(), end = d_vars.end();
      i == end; ++i){
    vars.push_back(i->exportTo(exprManager, variableMap));
  };
  /** Convert guards */
  VExpr guards; guards.reserve(d_guards.size());
  for(VExpr::iterator i = d_guards.begin(), end = d_guards.end();
      i == end; ++i){
    guards.push_back(i->exportTo(exprManager, variableMap));
  };
  /** Convert heads */
  VExpr heads; heads.reserve(d_heads.size());
  for(VExpr::iterator i = d_heads.begin(), end = d_heads.end();
      i == end; ++i){
    heads.push_back(i->exportTo(exprManager, variableMap));
  };
  /** Convert triggers */
  Triggers triggers; triggers.resize(d_triggers.size());
  for(size_t i = 0, end = d_triggers.size();
      i < end; ++i){
    triggers[i].reserve(d_triggers[i].size());
    for(VExpr::iterator j = d_triggers[i].begin(), jend = d_triggers[i].end();
        j == jend; ++i){
      triggers[i].push_back(j->exportTo(exprManager, variableMap));
    };
  };
  /** Convert head and body */
  Expr body = d_body.exportTo(exprManager, variableMap);
  /** Create the converted rules */
  return new PropagateRuleCommand(vars, guards, heads, body, triggers);
}

Command* PropagateRuleCommand::clone() const {
  return new PropagateRuleCommand(d_vars, d_guards, d_heads, d_body, d_triggers);
}

std::string PropagateRuleCommand::getCommandName() const throw() {
  return "propagate-rule";
}

/* output stream insertion operator for benchmark statuses */
std::ostream& operator<<(std::ostream& out,
                         BenchmarkStatus status) throw() {
  switch(status) {

  case SMT_SATISFIABLE:
    return out << "sat";

  case SMT_UNSATISFIABLE:
    return out << "unsat";

  case SMT_UNKNOWN:
    return out << "unknown";

  default:
    return out << "BenchmarkStatus::[UNKNOWNSTATUS!]";
  }
}

}/* CVC4 namespace */

#pragma once

#include <memory>
#include <string>

#include "Expression.hpp"

class Program;
class VarState;

class Statement {
 public:
  explicit Statement(std::string source);
  virtual ~Statement() = default;

  virtual void execute(VarState& state, Program& program) const = 0;

  const std::string& text() const noexcept;
  void set_text_for_list(const std::string& txt) { source_ = txt; }

 private:
  std::string source_;
};

class LetStatement : public Statement {
 public:
  LetStatement(std::string src, std::string var, Expression* expr);
  ~LetStatement();
  void execute(VarState& state, Program& program) const override;

 private:
  std::string var_;
  Expression* expr_;
};

class PrintStatement : public Statement {
 public:
  PrintStatement(std::string src, Expression* expr);
  ~PrintStatement();
  void execute(VarState& state, Program& program) const override;

 private:
  Expression* expr_;
};

class InputStatement : public Statement {
 public:
  InputStatement(std::string src, std::string var);
  void execute(VarState& state, Program& program) const override;

 private:
  std::string var_;
};

class GotoStatement : public Statement {
 public:
  GotoStatement(std::string src, int target);
  void execute(VarState& state, Program& program) const override;

 private:
  int target_;
};

class IfStatement : public Statement {
 public:
  IfStatement(std::string src, Expression* lhs, char op, Expression* rhs,
              int target);
  ~IfStatement();
  void execute(VarState& state, Program& program) const override;

 private:
  Expression* lhs_;
  Expression* rhs_;
  char op_;
  int target_;
};

class RemStatement : public Statement {
 public:
  RemStatement(std::string src);
  void execute(VarState& state, Program& program) const override;
};

class EndStatement : public Statement {
 public:
  EndStatement(std::string src);
  void execute(VarState& state, Program& program) const override;
};

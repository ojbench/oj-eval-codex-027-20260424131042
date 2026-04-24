#include "Statement.hpp"

#include <iostream>
#include <limits>
#include <sstream>
#include <utility>

#include "Program.hpp"
#include "VarState.hpp"
#include "utils/Error.hpp"

Statement::Statement(std::string source) : source_(std::move(source)) {}

const std::string& Statement::text() const noexcept { return source_; }

LetStatement::LetStatement(std::string src, std::string var, Expression* expr)
    : Statement(std::move(src)), var_(std::move(var)), expr_(expr) {}

LetStatement::~LetStatement() { delete expr_; }

void LetStatement::execute(VarState& state, Program&) const {
  int value = expr_->evaluate(state);
  state.setValue(var_, value);
}

PrintStatement::PrintStatement(std::string src, Expression* expr)
    : Statement(std::move(src)), expr_(expr) {}

PrintStatement::~PrintStatement() { delete expr_; }

void PrintStatement::execute(VarState& state, Program&) const {
  int value = expr_->evaluate(state);
  std::cout << value << "\n";
}

InputStatement::InputStatement(std::string src, std::string var)
    : Statement(std::move(src)), var_(std::move(var)) {}

void InputStatement::execute(VarState& state, Program&) const {
  std::cout << " ? ";
  std::cout.flush();
  std::string s;
  while (true) {
    if (!std::getline(std::cin, s)) {
      throw BasicError("INPUT ENDED");
    }
    // trim
    int i = 0;
    while (i < static_cast<int>(s.size()) && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    int j = static_cast<int>(s.size()) - 1;
    while (j >= 0 && std::isspace(static_cast<unsigned char>(s[j]))) --j;
    std::string t = (j >= i) ? s.substr(i, j - i + 1) : std::string();
    if (t.empty()) {
      std::cout << " ? ";
      std::cout.flush();
      continue;
    }
    bool neg = false;
    int pos = 0;
    if (t[0] == '+' || t[0] == '-') {
      neg = (t[0] == '-');
      pos = 1;
    }
    long long acc = 0;
    bool ok = true;
    for (int k = pos; k < static_cast<int>(t.size()); ++k) {
      char c = t[k];
      if (c >= '0' && c <= '9') {
        acc = acc * 10 + (c - '0');
        if (acc > static_cast<long long>(std::numeric_limits<int>::max()) + (neg ? 0LL : 1LL)) {
          ok = false;
          break;
        }
      } else {
        ok = false;
        break;
      }
    }
    if (!ok || (pos < static_cast<int>(t.size()) && pos == 0 && (t[0] == '+' || t[0] == '-') && t.size()==1)) {
      std::cout << "INVALID NUMBER\n";
      std::cout << " ? ";
      std::cout.flush();
      continue;
    }
    int val;
    if (neg) {
      if (acc == static_cast<long long>(std::numeric_limits<int>::max()) + 1LL) {
        val = std::numeric_limits<int>::min();
      } else if (acc <= static_cast<long long>(std::numeric_limits<int>::max())) {
        val = static_cast<int>(-acc);
      } else {
        std::cout << "INVALID NUMBER\n";
        std::cout << " ? ";
        std::cout.flush();
        continue;
      }
    } else {
      if (acc <= static_cast<long long>(std::numeric_limits<int>::max())) {
        val = static_cast<int>(acc);
      } else {
        std::cout << "INVALID NUMBER\n";
        std::cout << " ? ";
        std::cout.flush();
        continue;
      }
    }

    state.setValue(var_, val);
    break;
  }
}

GotoStatement::GotoStatement(std::string src, int target)
    : Statement(std::move(src)), target_(target) {}

void GotoStatement::execute(VarState&, Program& program) const {
  program.changePC(target_);
}

IfStatement::IfStatement(std::string src, Expression* lhs, char op,
                         Expression* rhs, int target)
    : Statement(std::move(src)), lhs_(lhs), rhs_(rhs), op_(op), target_(target) {}

IfStatement::~IfStatement() {
  delete lhs_;
  delete rhs_;
}

void IfStatement::execute(VarState& state, Program& program) const {
  int a = lhs_->evaluate(state);
  int b = rhs_->evaluate(state);
  bool cond = false;
  switch (op_) {
    case '=':
      cond = (a == b);
      break;
    case '<':
      cond = (a < b);
      break;
    case '>':
      cond = (a > b);
      break;
    default:
      throw BasicError("SYNTAX ERROR");
  }
  if (cond) {
    program.changePC(target_);
  }
}

RemStatement::RemStatement(std::string src) : Statement(std::move(src)) {}

void RemStatement::execute(VarState&, Program&) const { /* nop */ }

EndStatement::EndStatement(std::string src) : Statement(std::move(src)) {}

void EndStatement::execute(VarState&, Program& program) const {
  program.programEnd();
}

IndentStatement::IndentStatement(std::string src) : Statement(std::move(src)) {}

void IndentStatement::execute(VarState& state, Program&) const {
  state.pushScope();
}

DedentStatement::DedentStatement(std::string src) : Statement(std::move(src)) {}

void DedentStatement::execute(VarState& state, Program&) const { state.popScope(); }

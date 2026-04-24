#include "Program.hpp"

#include <iostream>

#include "Recorder.hpp"
#include "Statement.hpp"
#include "utils/Error.hpp"

Program::Program() : programCounter_(0), programEnd_(false) {}

void Program::addStmt(int line, Statement* stmt) { recorder_.add(line, stmt); }

void Program::removeStmt(int line) { recorder_.remove(line); }

void Program::run() {
  programEnd_ = false;
  // find smallest line
  int pc = recorder_.nextLine(-1);
  if (pc == -1) return;
  programCounter_ = pc;
  try {
    while (!programEnd_ && pc != -1) {
      const Statement* stmt = recorder_.get(pc);
      if (!stmt) {
        std::cout << "LINE NUMBER ERROR\n";
        return;
      }
      // execute may change PC
      const_cast<Statement*>(stmt)->execute(vars_, *this);
      if (programEnd_) break;
      if (programCounter_ == pc) {
        pc = recorder_.nextLine(pc);
        programCounter_ = pc;
      } else {
        // programCounter_ already set by changePC
        pc = programCounter_;
        if (pc != -1 && !recorder_.hasLine(pc)) {
          std::cout << "LINE NUMBER ERROR\n";
          return;
        }
      }
    }
  } catch (const BasicError& e) {
    std::cout << e.message() << "\n";
  }
}

void Program::list() const { recorder_.printLines(); }

void Program::clear() {
  recorder_.clear();
  vars_.clear();
}

void Program::execute(Statement* stmt) {
  try {
    stmt->execute(vars_, *this);
  } catch (const BasicError& e) {
    std::cout << e.message() << "\n";
  }
}

int Program::getPC() const noexcept { return programCounter_; }

void Program::changePC(int line) { programCounter_ = line; }

void Program::programEnd() { programEnd_ = true; }

void Program::resetAfterRun() noexcept { programEnd_ = false; }


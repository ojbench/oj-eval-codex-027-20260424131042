#include <iostream>
#include <memory>
#include <string>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Program.hpp"
#include "Token.hpp"
#include "utils/Error.hpp"

int main() {
  Lexer lexer;
  Parser parser;
  Program program;

  std::string line;
  while (std::getline(std::cin, line)) {
    if (line.empty()) {
      continue;
    }
    try {
      TokenStream ts = lexer.tokenize(line);
      const Token* first = ts.peek();
      if (!first) continue;

      // Interpreter commands handled directly
      switch (first->type) {
        case TokenType::RUN:
          program.run();
          continue;
        case TokenType::LIST:
          program.list();
          continue;
        case TokenType::CLEAR:
          program.clear();
          continue;
        case TokenType::QUIT:
          return 0;
        case TokenType::HELP:
          std::cout << "LET, PRINT, INPUT, END, REM, GOTO, IF, THEN, RUN, LIST, CLEAR, QUIT, HELP\n";
          continue;
        default:
          break;
      }

      // Parse as program line or immediate statement
      ParsedLine pl = parser.parseLine(ts, line);
      auto ln = pl.getLine();
      if (ln.has_value()) {
        Statement* s = pl.fetchStatement();
        if (s == nullptr) {
          program.removeStmt(*ln);
        } else {
          program.addStmt(*ln, s);
        }
      } else {
        Statement* s = pl.fetchStatement();
        if (!s) continue;
        program.execute(s);
        delete s;
      }
    } catch (const BasicError& e) {
      std::cout << e.message() << "\n";
    }
  }
  return 0;
}

#include "Parser.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Expression.hpp"
#include "Statement.hpp"
#include "utils/Error.hpp"

ParsedLine::ParsedLine() { statement_ = nullptr; }

ParsedLine::~ParsedLine() { delete statement_; }

void ParsedLine::setLine(int line) { line_number_.emplace(line); }

std::optional<int> ParsedLine::getLine() { return line_number_; }

void ParsedLine::setStatement(Statement* stmt) { statement_ = stmt; }

Statement* ParsedLine::getStatement() const { return statement_; }

Statement* ParsedLine::fetchStatement() {
  Statement* temp = statement_;
  statement_ = nullptr;
  return temp;
}

ParsedLine Parser::parseLine(TokenStream& tokens, const std::string& originLine) const {
  ParsedLine result;

  const Token* firstToken = tokens.peek();
  if (firstToken && firstToken->type == TokenType::NUMBER) {
    int ln = parseLiteral(firstToken);
    result.setLine(ln);
    tokens.get();
    if (tokens.empty()) {
      return result;
    }
  }

  Statement* s = parseStatement(tokens, originLine);
  if (s) {
    // derive printable text without the line number and trim leading spaces
    size_t start = 0;
    // tokens.data() holds all tokens; the first token was the line number we consumed.
    // Its column marks end position of that token.
    if (!tokens.data().empty()) {
      start = static_cast<size_t>(tokens.data()[0].column);
    } else {
      start = originLine.size();
    }
    if (start > originLine.size()) start = originLine.size();
    std::string stmt_text = originLine.substr(start);
    // left trim spaces
    size_t p = stmt_text.find_first_not_of(" \t\r\n");
    if (p == std::string::npos) stmt_text.clear();
    else if (p > 0) stmt_text.erase(0, p);
    s->set_text_for_list(stmt_text);
  }
  result.setStatement(s);
  return result;
}

Statement* Parser::parseStatement(TokenStream& tokens, const std::string& originLine) const {
  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }
  const Token* token = tokens.get();
  if (!token) {
    throw BasicError("SYNTAX ERROR");
  }
  switch (token->type) {
    case TokenType::LET:
      return parseLet(tokens, originLine);
    case TokenType::PRINT:
      return parsePrint(tokens, originLine);
    case TokenType::INPUT:
      return parseInput(tokens, originLine);
    case TokenType::GOTO:
      return parseGoto(tokens, originLine);
    case TokenType::IF:
      return parseIf(tokens, originLine);
    case TokenType::REM:
      return parseRem(tokens, originLine);
    case TokenType::END:
      return parseEnd(tokens, originLine);
    case TokenType::INDENT:
      return parseIndent(tokens, originLine);
    case TokenType::DEDENT:
      return parseDedent(tokens, originLine);
    default:
      throw BasicError("SYNTAX ERROR");
  }
}

Statement* Parser::parseLet(TokenStream& tokens, const std::string& originLine) const {
  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }
  const Token* varToken = tokens.get();
  if (!varToken || varToken->type != TokenType::IDENTIFIER) {
    throw BasicError("SYNTAX ERROR");
  }
  std::string varName = varToken->text;
  if (tokens.empty() || tokens.get()->type != TokenType::EQUAL) {
    throw BasicError("SYNTAX ERROR");
  }
  auto expr = parseExpression(tokens);
  return new LetStatement(originLine, varName, expr);
}

Statement* Parser::parsePrint(TokenStream& tokens, const std::string& originLine) const {
  auto expr = parseExpression(tokens);
  return new PrintStatement(originLine, expr);
}

Statement* Parser::parseInput(TokenStream& tokens, const std::string& originLine) const {
  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }
  const Token* varToken = tokens.get();
  if (!varToken || varToken->type != TokenType::IDENTIFIER) {
    throw BasicError("SYNTAX ERROR");
  }
  std::string varName = varToken->text;
  return new InputStatement(originLine, varName);
}

Statement* Parser::parseGoto(TokenStream& tokens, const std::string& originLine) const {
  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }
  const Token* lineToken = tokens.get();
  if (!lineToken || lineToken->type != TokenType::NUMBER) {
    throw BasicError("SYNTAX ERROR");
  }
  int targetLine = parseLiteral(lineToken);
  return new GotoStatement(originLine, targetLine);
}

Statement* Parser::parseIf(TokenStream& tokens, const std::string& originLine) const {
  auto leftExpr = parseExpression(tokens);
  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }
  const Token* opToken = tokens.get();
  char op;
  switch (opToken->type) {
    case TokenType::EQUAL:
      op = '=';
      break;
    case TokenType::GREATER:
      op = '>';
      break;
    case TokenType::LESS:
      op = '<';
      break;
    default:
      throw BasicError("SYNTAX ERROR");
  }
  auto rightExpr = parseExpression(tokens);
  if (tokens.empty() || tokens.get()->type != TokenType::THEN) {
    throw BasicError("SYNTAX ERROR");
  }
  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }
  const Token* lineToken = tokens.get();
  if (!lineToken || lineToken->type != TokenType::NUMBER) {
    throw BasicError("SYNTAX ERROR");
  }
  int targetLine = parseLiteral(lineToken);
  return new IfStatement(originLine, leftExpr, op, rightExpr, targetLine);
}

Statement* Parser::parseRem(TokenStream& tokens, const std::string& originLine) const {
  const Token* remInfo = tokens.get();
  if (!remInfo || remInfo->type != TokenType::REMINFO) {
    throw BasicError("SYNTAX ERROR");
  }
  (void)remInfo;  // ignore content
  return new RemStatement(originLine);
}

Statement* Parser::parseEnd(TokenStream& tokens, const std::string& originLine) const {
  (void)tokens;
  return new EndStatement(originLine);
}

Statement* Parser::parseIndent(TokenStream& tokens, const std::string& originLine) const {
  (void)tokens;
  return new IndentStatement(originLine);
}

Statement* Parser::parseDedent(TokenStream& tokens, const std::string& originLine) const {
  (void)tokens;
  return new DedentStatement(originLine);
}

Expression* Parser::parseExpression(TokenStream& tokens) const { return parseExpression(tokens, 0); }

Expression* Parser::parseExpression(TokenStream& tokens, int precedence) const {
  Expression* left;
  if (tokens.empty()) {
    throw BasicError("SYNTAX ERROR");
  }
  const Token* token = tokens.get();
  if (!token) {
    throw BasicError("SYNTAX ERROR");
  }
  if (token->type == TokenType::NUMBER) {
    int value = parseLiteral(token);
    left = new ConstExpression(value);
  } else if (token->type == TokenType::IDENTIFIER) {
    left = new VariableExpression(token->text);
  } else if (token->type == TokenType::LEFT_PAREN) {
    ++leftParentCount;
    left = parseExpression(tokens, 0);
    if (tokens.empty() || tokens.get()->type != TokenType::RIGHT_PAREN) {
      throw BasicError("MISMATCHED PARENTHESIS");
    }
    --leftParentCount;
  } else {
    throw BasicError("SYNTAX ERROR");
  }
  while (!tokens.empty()) {
    const Token* opToken = tokens.peek();
    if (!opToken) break;
    if (opToken->type == TokenType::RIGHT_PAREN) {
      if (leftParentCount == 0) {
        throw BasicError("MISMATCHED PARENTHESIS");
      }
      break;
    }
    int opPrecedence = getPrecedence(opToken->type);
    if (opPrecedence == -1 || opPrecedence < precedence) break;
    tokens.get();
    char op;
    switch (opToken->type) {
      case TokenType::PLUS:
        op = '+';
        break;
      case TokenType::MINUS:
        op = '-';
        break;
      case TokenType::MUL:
        op = '*';
        break;
      case TokenType::DIV:
        op = '/';
        break;
      default:
        throw BasicError("SYNTAX ERROR");
    }
    auto right = parseExpression(tokens, opPrecedence + 1);
    left = new CompoundExpression(left, op, right);
  }
  return left;
}

int Parser::getPrecedence(TokenType op) const {
  switch (op) {
    case TokenType::PLUS:
    case TokenType::MINUS:
      return 1;
    case TokenType::MUL:
    case TokenType::DIV:
      return 2;
    default:
      return -1;
  }
}

int Parser::parseLiteral(const Token* token) const {
  if (!token || token->type != TokenType::NUMBER) {
    throw BasicError("SYNTAX ERROR");
  }
  try {
    size_t pos;
    int value = std::stoi(token->text, &pos);
    if (pos != token->text.length()) {
      throw BasicError("INT LITERAL OVERFLOW");
    }
    return value;
  } catch (const std::out_of_range&) {
    throw BasicError("INT LITERAL OVERFLOW");
  } catch (const std::invalid_argument&) {
    throw BasicError("SYNTAX ERROR");
  }
}

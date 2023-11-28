#pragma once

#include <iosfwd>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

class TestRunner;

namespace Parse {

namespace TokenType {
struct Number {
  int value;
};

struct Id {
  std::string value;
};

struct Char {
  char value;
};

struct String {
  std::string value;
};

struct Class {};
struct Return {};
struct If {};
struct Else {};
struct Def {};
struct Newline {};
struct Print {};
struct Indent {};
struct Dedent {};
struct Eof {};
struct And {};
struct Or {};
struct Not {};
struct Eq {};
struct NotEq {};
struct LessOrEq {};
struct GreaterOrEq {};
struct None {};
struct True {};
struct False {};
} // namespace TokenType

using TokenBase =
    std::variant<TokenType::Number, TokenType::Id, TokenType::Char,
                 TokenType::String, TokenType::Class, TokenType::Return,
                 TokenType::If, TokenType::Else, TokenType::Def,
                 TokenType::Newline, TokenType::Print, TokenType::Indent,
                 TokenType::Dedent, TokenType::And, TokenType::Or,
                 TokenType::Not, TokenType::Eq, TokenType::NotEq,
                 TokenType::LessOrEq, TokenType::GreaterOrEq, TokenType::None,
                 TokenType::True, TokenType::False, TokenType::Eof>;

// По сравнению с условием задачи мы добавили в тип Token несколько
// удобных методов, которые делают код короче. Например,
//
// token.Is<TokenType::Id>()
//
// гораздо короче, чем
//
// std::holds_alternative<TokenType::Id>(token).
struct Token : TokenBase {
  using TokenBase::TokenBase;

  template <typename T> [[nodiscard]] bool Is() const {
    return std::holds_alternative<T>(*this);
  }

  template <typename T> const T &As() const { return std::get<T>(*this); }

  template <typename T> const T *TryAs() const { return std::get_if<T>(this); }
};

bool operator==(const Token &lhs, const Token &rhs);

bool operator!=(const Token &lhs, const Token &rhs);

std::ostream &operator<<(std::ostream &os, const Token &rhs);

class LexerError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class Lexer {
public:
  explicit Lexer(std::istream &input);

  [[nodiscard]] const Token &CurrentToken() const;

  Token NextToken();

  template <typename T> const T &Expect() const {
    if (CurrentToken().Is<T>()) {
      return CurrentToken().As<T>();
    }

    throw LexerError("Expectation Error!");
  }

  template <typename T, typename U> void Expect(const U &value) const {
    if (!CurrentToken().Is<T>() || CurrentToken().As<T>().value != value) {
      throw LexerError("Expectation Error!");
    }
  }

  template <typename T> const T &ExpectNext() {
    NextToken();
    return Expect<T>();
  }

  template <typename T, typename U> void ExpectNext(const U &value) {
    NextToken();
    Expect<T>(value);
  }

private:
  class IndentController {
  public:
    IndentController() = default;
    ;

    void IndentNext() { indent_next_ = true; }

    void NotIndentNext() { indent_next_ = false; }

    [[nodiscard]] bool IsIndentNext() const { return indent_next_; }

    [[nodiscard]] bool NeedNewLine() const { return need_new_line_; }

    void SetNewLine() { need_new_line_ = false; }

    std::optional<Token> UpdateIndent(std::istream &);

  private:
    bool indent_next_ = true;
    unsigned current_indents_ = 0;
    unsigned needed_indents_ = 0;
    unsigned needed_dedents_ = 0;
    bool need_new_line_ = true;

    void UpdateNext();
  };

  std::istream &in_;
  Token current_token_;
  IndentController indent_controller_;

  Token ReadToken();

  TokenType::Number ReadNumber();

  TokenType::String ReadString();

  std::string ReadStringToEnd(const char &end);

  Token ReadIdOrDeclaredToken();

  [[nodiscard]] bool IsBoolToken(const char &c) const;

  Token ReadBoolToken();

  const std::unordered_map<std::string, Token> declared_tokens_ = {
      {"class", TokenType::Class{}}, {"return", TokenType::Return{}},
      {"if", TokenType::If{}},       {"else", TokenType::Else{}},
      {"def", TokenType::Def{}},     {"print", TokenType::Print{}},
      {"and", TokenType::And{}},     {"or", TokenType::Or{}},
      {"not", TokenType::Not{}},     {"None", TokenType::None{}},
      {"True", TokenType::True{}},   {"False", TokenType::False{}}};

  const std::unordered_map<std::string, Token> bool_tokens_ = {
      {"==", TokenType::Eq{}},
      {"!=", TokenType::NotEq{}},
      {"<=", TokenType::LessOrEq{}},
      {">=", TokenType::GreaterOrEq{}}};

  const std::unordered_set<char> char_tokens_ = {
      '+', '-', '*', '/', '%', '.', ',', '=', ':', '?',
      '!', '<', '>', '(', ')', '[', ']', '{', '}'};

  [[nodiscard]] bool IsCharToken(const char &c) const;
};

void RunLexerTests(TestRunner &test_runner);

} /* namespace Parse */

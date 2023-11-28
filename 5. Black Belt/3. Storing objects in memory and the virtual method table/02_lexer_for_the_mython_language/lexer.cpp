#include "lexer.h"

#include <charconv>
#include <unordered_map>

using namespace std;

namespace Parse {

    bool operator==(const Token &lhs, const Token &rhs) {
        using namespace TokenType;

        if (lhs.index() != rhs.index()) {
            return false;
        }
        if (lhs.Is<Char>()) {
            return lhs.As<Char>().value == rhs.As<Char>().value;
        } else if (lhs.Is<Number>()) {
            return lhs.As<Number>().value == rhs.As<Number>().value;
        } else if (lhs.Is<String>()) {
            return lhs.As<String>().value == rhs.As<String>().value;
        } else if (lhs.Is<Id>()) {
            return lhs.As<Id>().value == rhs.As<Id>().value;
        } else {
            return true;
        }
    }

    bool operator!=(const Token &lhs, const Token &rhs) {
        return !(lhs == rhs);
    }

    std::ostream &operator<<(std::ostream &os, const Token &rhs) {
        using namespace TokenType;

#define VALUED_OUTPUT(type)                                                    \
  if (auto p = rhs.TryAs<type>())                                              \
    return os << #type << '{' << p->value << '}';

        VALUED_OUTPUT(Number);
        VALUED_OUTPUT(Id);
        VALUED_OUTPUT(String);
        VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type)                                                  \
  if (rhs.Is<type>())                                                          \
    return os << #type;

        UNVALUED_OUTPUT(Class);
        UNVALUED_OUTPUT(Return);
        UNVALUED_OUTPUT(If);
        UNVALUED_OUTPUT(Else);
        UNVALUED_OUTPUT(Def);
        UNVALUED_OUTPUT(Newline);
        UNVALUED_OUTPUT(Print);
        UNVALUED_OUTPUT(Indent);
        UNVALUED_OUTPUT(Dedent);
        UNVALUED_OUTPUT(And);
        UNVALUED_OUTPUT(Or);
        UNVALUED_OUTPUT(Not);
        UNVALUED_OUTPUT(Eq);
        UNVALUED_OUTPUT(NotEq);
        UNVALUED_OUTPUT(LessOrEq);
        UNVALUED_OUTPUT(GreaterOrEq);
        UNVALUED_OUTPUT(None);
        UNVALUED_OUTPUT(True);
        UNVALUED_OUTPUT(False);
        UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

        return os << "Unknown token :(";
    }

    Lexer::Lexer(std::istream &input) : in_(input) {
        current_token_ = ReadToken();
        indent_controller_.NotIndentNext();
    }

    const Token &Lexer::CurrentToken() const {
        return current_token_;
    }

    Token Lexer::NextToken() {
        current_token_ = ReadToken();
        return current_token_;
    }

    Token Lexer::ReadToken() {
        if (indent_controller_.IsIndentNext()) {
            const auto result = indent_controller_.UpdateIndent(in_);
            if (result) {
                return *result;
            }
            return ReadToken();
        }

        char c = in_.get();

        if (in_.eof()) {
            if (indent_controller_.NeedNewLine()) {
                indent_controller_.SetNewLine();
                return TokenType::Newline{};
            }
            return TokenType::Eof{};
        }

        if (c == '\n') {
            indent_controller_.IndentNext();
            indent_controller_.SetNewLine();
            return TokenType::Newline{};
        }

        if (std::isdigit(c)) {
            in_.putback(c);
            return ReadNumber();
        }

        if (IsCharToken(c)) {
            return TokenType::Char{c};
        }

        if (c == '\'' || c == '\"') {
            in_.putback(c);
            return ReadString();
        }

        if (c == ' ') {
            return ReadToken();
        }

        in_.putback(c);
        return ReadIdOrDeclaredToken();
    }

    TokenType::Number Lexer::ReadNumber() {
        int result = 0;
        while (std::isdigit(in_.peek())) {
            if (result > (std::numeric_limits<int>::max() - (in_.peek() - '0')) / 10) {
                throw LexerError("Number too large!");
            }
            result *= 10;
            result += in_.get() - '0';
        }
        return TokenType::Number{result};
    }

    TokenType::String Lexer::ReadString() {
        return TokenType::String{ReadStringToEnd(in_.get())};
    }

    std::string Lexer::ReadStringToEnd(const char &end) {
        std::string result;
        std::getline(in_, result, end);
        return result;
    }

    Token Lexer::ReadBoolToken() {
        std::string str;
        str += in_.get();
        str += in_.get();
        if (!bool_tokens_.count(str)) {
            throw LexerError("Invalid two-char token!");
        }
        return bool_tokens_.at(str);
    }

    Token Lexer::ReadIdOrDeclaredToken() {
        char c = in_.get();
        if (IsBoolToken(c)) {
            in_.putback(c);
            return ReadBoolToken();
        }
        in_.putback(c);

        std::string s;
        while (
                in_.peek() != ' ' && in_.peek() != '\n' &&
                in_.peek() != -1 && !IsCharToken(in_.peek())
                ) {
            s += in_.get();
        }

        const auto finder = declared_tokens_.find(s);
        if (finder != declared_tokens_.end()) {
            return finder->second;
        }

        return TokenType::Id{s};
    }

    bool Lexer::IsBoolToken(const char &c) const {
        return (c == '=' || c == '!' || c == '<' || c == '>') && (in_.peek() == '=');
    }

    bool Lexer::IsCharToken(const char &c) const {
        return (char_tokens_.count(c) && !IsBoolToken(c));
    }

    void Lexer::IndentController::UpdateNext() {
        if (needed_indents_ == 0 && needed_dedents_ == 0) {
            indent_next_ = false;
        }
    }

    std::optional<Token> Lexer::IndentController::UpdateIndent(std::istream &in) {
        if (needed_indents_ > 0) {
            needed_indents_--;
            current_indents_++;
            UpdateNext();
            return TokenType::Indent{};
        }

        if (needed_dedents_ > 0) {
            needed_dedents_--;
            current_indents_--;
            UpdateNext();
            return TokenType::Dedent{};
        }

        unsigned spaces = 0;
        while (in.peek() == ' ') {
            in.get();
            spaces++;
        }

        if (in.peek() == '\n') {
            in.get();
            return std::nullopt;
        } else if (in.peek() == -1) {
            SetNewLine();
        }

        spaces = (spaces + 1) / 2;

        if (spaces == current_indents_) {
            indent_next_ = false;
            return std::nullopt;
        } else if (spaces > current_indents_) {
            needed_indents_ = spaces - current_indents_;
            needed_indents_--;
            current_indents_++;
            UpdateNext();
            return TokenType::Indent{};
        } else {
            needed_dedents_ = current_indents_ - spaces;
            needed_dedents_--;
            current_indents_--;
            UpdateNext();
            return TokenType::Dedent{};
        }
    }

} /* namespace Parse */

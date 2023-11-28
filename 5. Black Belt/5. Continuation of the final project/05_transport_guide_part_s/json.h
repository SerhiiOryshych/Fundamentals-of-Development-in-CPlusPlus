#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace Json {

    class Node;

    using Array = std::vector<Node>;
    using Dict = std::map<std::string, Node>;

    class Node : std::variant<Array, Dict, bool, int, double, std::string> {
    public:
        using variant::variant;

        [[nodiscard]] const variant &GetBase() const { return *this; }

        [[nodiscard]] bool IsArray() const {
            return std::holds_alternative<Array>(*this);
        }

        [[nodiscard]] const auto &AsArray() const {
            return std::get<Array>(*this);
        }

        [[nodiscard]] bool IsMap() const {
            return std::holds_alternative<Dict>(*this);
        }

        [[nodiscard]] const auto &AsMap() const {
            return std::get<Dict>(*this);
        }

        [[nodiscard]] bool IsBool() const {
            return std::holds_alternative<bool>(*this);
        }

        [[nodiscard]] bool AsBool() const {
            return std::get<bool>(*this);
        }

        [[nodiscard]] bool IsInt() const {
            return std::holds_alternative<int>(*this);
        }

        [[nodiscard]] int AsInt() const {
            return std::get<int>(*this);
        }

        [[nodiscard]] bool IsPureDouble() const {
            return std::holds_alternative<double>(*this);
        }

        [[nodiscard]] bool IsDouble() const {
            return IsPureDouble() || IsInt();
        }

        [[nodiscard]] double AsDouble() const {
            return IsPureDouble() ? std::get<double>(*this) : AsInt();
        }

        [[nodiscard]] bool IsString() const {
            return std::holds_alternative<std::string>(*this);
        }

        [[nodiscard]] const auto &AsString() const {
            return std::get<std::string>(*this);
        }
    };

    class Document {
    public:
        explicit Document(Node root) : root(std::move(root)) {}

        [[nodiscard]] const Node &GetRoot() const {
            return root;
        }

    private:
        Node root;
    };

    Node LoadNode(std::istream &input);

    Document Load(std::istream &input);

    void PrintNode(const Node &node, std::ostream &output);

    template<typename Value>
    void PrintValue(const Value &value, std::ostream &output) {
        output << value;
    }

    template<>
    void PrintValue<std::string>(const std::string &value, std::ostream &output);

    template<>
    void PrintValue<bool>(const bool &value, std::ostream &output);

    template<>
    void PrintValue<Array>(const Array &nodes, std::ostream &output);

    template<>
    void PrintValue<Dict>(const Dict &dict, std::ostream &output);

    void Print(const Document &document, std::ostream &output);

}


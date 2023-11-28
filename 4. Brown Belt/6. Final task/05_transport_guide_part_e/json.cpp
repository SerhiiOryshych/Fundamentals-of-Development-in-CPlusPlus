#include "json.h"

using namespace std;

namespace Json {

    Document::Document(Node root) : root(move(root)) {}

    const Node &Document::GetRoot() const { return root; }

    Node LoadNode(istream &input);

    Node LoadArray(istream &input) {
        vector<Node> result;

        for (char c; input >> c && c != ']';) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }

        return Node(move(result));
    }

    Node LoadInt(istream &input) {
        int result = 0;
        while (isdigit(input.peek())) {
            result *= 10;
            result += input.get() - '0';
        }
        return Node(result);
    }

    Node LoadDouble(const string &str) { return Node((double) stod(str)); }

    Node LoadBool(const string &str) { return Node((bool) (str == "true")); }

    Node LoadString(istream &input) {
        string line;
        getline(input, line, '"');
        return Node(move(line));
    }

    Node LoadWord(istream &input) {
        string line = "";
        char ch;
        while (input >> ch) {
            if (ch == ',' || ch == '}') {
                input.putback(ch);
                break;
            }
            line += ch;
        }
        return Node(move(line));
    }

    Node LoadDict(istream &input) {
        map<string, Node> result;

        for (char c; input >> c && c != '}';) {
            if (c == ',') {
                input >> c;
            }

            string key = LoadString(input).AsString();
            input >> c;
            result.emplace(move(key), LoadNode(input));
        }

        return Node(move(result));
    }

    Node LoadNode(istream &input) {
        char c;
        input >> c;

        if (c == '[') {
            return LoadArray(input);
        } else if (c == '{') {
            return LoadDict(input);
        } else if (c == '"') {
            return LoadString(input);
        } else {
            input.putback(c);
            auto node = LoadWord(input);
            auto node_as_string = node.AsString();
            if (node_as_string.back() == ',') {
                node_as_string = node_as_string.substr(0, node_as_string.size() - 1);
            }
            if (node_as_string == "true" || node_as_string == "false") {
                return LoadBool(node_as_string);
            } else {
                return LoadDouble(node_as_string);
            }
        }
    }

    Document Load(istream &input) { return Document{LoadNode(input)}; }

} // namespace Json

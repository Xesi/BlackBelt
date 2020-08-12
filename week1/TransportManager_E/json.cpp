#include "json.h"

using namespace std;

namespace Json {

    Document::Document(Node root) : root(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root;
    }

    Node LoadNode(istream& input);

    Node LoadArray(istream& input) {
        vector<Node> result;

        for (char c; input >> c && c != ']'; ) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }

        return Node(move(result));
    }

    Node LoadNumber(istream& input) {
        int main_part = 0;
        bool isNegat = false;
        if (input.peek() == '-') {
            input.get();
            isNegat = true;
        }
        while(isdigit(input.peek())) {
            main_part *= 10;
            main_part += input.get() - '0';
        }
        if (input.peek() != '.') {
            if (isNegat) main_part *= -1;
            return Node(main_part);
        }
        input.get();
        double number = main_part;
        double power = 1.0;
        while (isdigit(input.peek())) {
            power /= 10;
            number += power * (input.get() - '0');
        }
        if (isNegat) number *= -1;
        return Node(number);
    }
    Node LoadBool(istream& input) {
        string s;
        while (input.peek() >= 'a' && input.peek() <= 'z') {
            s += input.get();
        }
        return (s == "true" ? Node(true) : Node(false));
    }

    Node LoadString(istream& input) {
        string line;
        getline(input, line, '"');
        return Node(move(line));
    }

    Node LoadDict(istream& input) {
        map<string, Node> result;

        for (char c; input >> c && c != '}'; ) {
            if (c == ',') {
                input >> c;
            }

            string key = LoadString(input).AsString();
            input >> c;
            result.emplace(move(key), LoadNode(input));
        }

        return Node(move(result));
    }

    Node LoadNode(istream& input) {
        char c;
        input >> c;

        if (c == '[') {
            return LoadArray(input);
        }
        else if (c == '{') {
            return LoadDict(input);
        }
        else if (c == '"') {
            return LoadString(input);
        }
        else {
            while (c == ' ') c = input.get();
            if (isdigit(c) || c == '-') {
                input.putback(c);
                return LoadNumber(input);
            }
            else {
                input.putback(c);
                return LoadBool(input);
            }
        }
    }
    
    void Node::PrintNode(ostream& output) const {
        if (std::holds_alternative<int>(*this)) {
            output << std::get<int>(*this);
        }
        else if (std::holds_alternative<double>(*this)) {
            output << std::fixed << std::setprecision(8) << std::get<double>(*this);
        }
        else if (std::holds_alternative<bool>(*this)) {
            bool value = std::get<bool>(*this);
            output << (value ? "true" : "false");
        }
        else if (std::holds_alternative<std::string>(*this)) {
            output << "\"" << std::get<string>(*this) << "\"";
        }
        else if (std::holds_alternative<std::vector<Node>>(*this)) {
            output << "[\n";
            const std::vector<Node>& vec = std::get<std::vector<Node>>(*this);
            for (size_t i = 0; i < vec.size(); ++i) {
                vec[i].PrintNode(output);
                output << (i != vec.size() - 1 ? ",\n" : "\n");
            }
            output << "]\n";
        }
        else if (std::holds_alternative<std::map<std::string, Node>>(*this)) {
            output << "{\n";
            const std::map<std::string, Node> & mp = std::get<std::map<std::string, Node>>(*this);
            
            for (auto it = mp.begin(); it != mp.end(); ++it) {
                auto& p = *it;
                output << "\"" << p.first << "\": ";
                p.second.PrintNode(output);
                output << (next(it) == mp.end() ? "\n" : ",\n");
            }
            output << "}\n";
        }
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(ostream& output, const Document& document) {
        document.GetRoot().PrintNode(output);
    }

}
#pragma once

#include <istream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <iostream>
#include <iomanip>

namespace Json {

    class Node : std::variant<std::vector<Node>,
        std::map<std::string, Node>,
        int, double, bool, 
        std::string> {
    public:
        using variant::variant;

        const auto& AsArray() const {
            return std::get<std::vector<Node>>(*this);
        }
        const auto& AsMap() const {
            return std::get<std::map<std::string, Node>>(*this);
        }
        int AsInt() const {
            return std::get<int>(*this);
        }
        bool AsBool() const {
            return std::get<bool>(*this);
        }
        double AsDouble() const {
            if (std::holds_alternative<double>(*this))
                return std::get<double>(*this);
            else return double(this->AsInt());
        }
        const auto& AsString() const {
            return std::get<std::string>(*this);
        }
        void PrintNode(std::ostream& output) const;

    };

    class Document {
      public:
          explicit Document(Node root);
          const Node& GetRoot() const;

      private:
        Node root;
    };

    Document Load(std::istream& input);
    void Print(std::ostream& output, const Document& document);

}
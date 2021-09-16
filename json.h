#pragma once

#include <ios>
#include <iostream>
#include <initializer_list>
#include <map>
#include <string>
#include <sstream>
#include <variant>
#include <vector>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};
Node LoadArray(std::istream& input);
Node LoadInt(std::istream& input);
Node LoadString(std::istream& input);
Node LoadDict(std::istream& input);
Node LoadBool(std::istream& input);

using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input);
Node LoadNode(std::istream& input);
Node LoadNull(std::istream& input);

using Variant = std::variant<std::nullptr_t, Array, Dict, int, double, bool, std::string>;

class Node {
public:
    Node() = default;
    Node(std::nullptr_t);
    Node(Array array);
    Node(Dict map);
    Node(int value);
    Node(double value);
    Node(bool value);
    Node(std::string value);

    bool IsNull() const;
    bool IsArray() const;
    const Array& AsArray() const;
    bool IsMap() const;
    const Dict& AsMap() const;
    bool IsInt() const;
    int AsInt() const;
    bool IsBool() const;
    bool AsBool() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    double AsDouble() const;
    bool IsString() const;
    const std::string& AsString() const;
    const Variant& GetNode()const;


private:
    Variant node_;
};

struct NodePrinter {
    std::ostream& out;

    void operator()();

    void operator()(std::nullptr_t);

    void operator()(Array array);

    void operator()(Dict dict);

    void operator()(int integer);

    void operator()(std::string str);

    void operator()(bool boolean);

    void operator()(double duble);

};

bool operator==(const Node& node1, const Node& node2);
bool operator!=(const Node& node1, const Node& node2);

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(const Document& doc1, const Document& doc2);
bool operator!=(const Document& doc1, const Document& doc2);
Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
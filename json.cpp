#include "json.h"

using namespace std;

namespace json {

Node LoadArray(istream& input) {
    Array result;
    char c;
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (c != ']'){
        throw ParsingError("No closing bracket in array");
    }
    return Node(move(result));
}

Node LoadInt(istream& input) {
    auto result = LoadNumber(input);
    if (std::holds_alternative<int>(result)){
        return Node(std::get<int>(result));
    } else {
        return Node(std::get<double>(result));
    }
}


Node LoadString(istream& input) {
    using namespace std::literals;
    string s;
    input >> std::noskipws;
    bool is_closed = false;
    for (char c; input >> c;){
        if (c == '\\') {
            input.get(c);
            if (c == EOF) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = c;
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (c == '"'){
            if (!s.empty()){
                if (s.back() != '\\'){
                    is_closed = true;
                    break ;
                } else {
                    s.pop_back();
                }
            }
        } else {
            s.push_back(c);
        }
    }
    if (!is_closed){
        input >> std::skipws;
        throw ParsingError("String not closed with '\"'");
    }
    input >> std::skipws;
    return Node(move(s));
}


Node LoadDict(istream& input) {
    Dict result;
    char c;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }
        string key = LoadString(input).AsString();
        input >> c;
        std::string line;
        result.insert({move(key), LoadNode(input)});
    }
    if (c != '}'){
        throw ParsingError("No closing bracket in map");
    }    
    return Node(move(result));
}

Node LoadNull(istream& input){
    using namespace std::literals;
    string str = "";
    for (char c; input >> c;){
        if (c == ','){
            break ;
        }
        str.push_back(c);
    }
    if (str == "null"s){
        return Node();
    } else {
        throw ParsingError("Invalid null argument");
    }
}

Node LoadNode(istream& input) {
    char c;
    input >> c;
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else {
        input.putback(c);
        return LoadInt(input);
    }
}

Node LoadBool(istream& input){
    using namespace std::literals;
    string str = "";
    for (char c; input >> c;){
        if (c == ',' || c == '}'){
            input.putback(c);
            break ;
        }
        str.push_back(c);
    }
    if (str == "true"){
        return Node(true);
    } else if (str == "false"){
        return Node(false);
    } else {
        throw ParsingError("Invalid true/false argument");
    }
}

Node::Node(std::nullptr_t)
    : node_(nullptr) {
}
Node::Node(int val)
    : node_(val) {
}
Node::Node(double val)
    : node_(val) {
}
Node::Node(std::string val)
    : node_(std::move(val)) {
}
Node::Node(Array val)
    : node_(std::move(val)) {
}
Node::Node(Dict val)
    : node_(std::move(val)) {
}
Node::Node(bool val)
    : node_(val) {
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(node_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(node_);
}

const Array& Node::AsArray() const {
    if (IsArray()){
        return std::get<Array>(node_);
    } else {
        throw std::logic_error("Value is not an array");
    }
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(node_);
}

const Dict& Node::AsMap() const {
    if (IsMap()){
        return std::get<Dict>(node_);
    } else {
        throw std::logic_error("Value is not map");
    }
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(node_);
}

int Node::AsInt() const {
    if (IsInt()){
        return std::get<int>(node_);
    } else {
        throw std::logic_error("Value is not int");
    }
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(node_);
}

bool Node::AsBool() const {
    if (IsBool()){ 
        return std::get<bool>(node_);
    } else {
        throw std::logic_error("Value is not bool");
    }
}

bool Node::IsDouble() const{
    return std::holds_alternative<double>(node_) || std::holds_alternative<int>(node_);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(node_);
}

double Node::AsDouble() const {
    if (IsInt()){
        return static_cast<double>(std::get<int>(node_));
    } else if (IsDouble()){
        return std::get<double>(node_);
    } else {
        throw std::logic_error("Value is not double nor int");
    }
}

const std::string& Node::AsString() const {
    if (IsString()){
        return std::get<std::string>(node_);
    } else {
        throw std::logic_error("Value is not string");
    }
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(node_);
}

const Variant& Node::GetNode()const{
    return node_;
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    // LOG_DURATION("JSON_LOAD");
    return Document{LoadNode(input)};
}

void NodePrinter::operator()(){
    using namespace std::literals;
    out << "null"sv << '\n';
}

void NodePrinter::operator()(std::nullptr_t){
    using namespace std::literals;
    out << "null"sv << '\n';
}

void NodePrinter::operator()(Array array){
    out << '[' << '\n';
    bool begin = true;
    for (const auto& elem : array){
        if (!begin){
            out << ',' << '\n';
        }
        std::visit(NodePrinter{out}, elem.GetNode());
        begin = false;
    }
    out << ']' << '\n';
}

void NodePrinter::operator()(Dict dict){
    out << '{'<< '\n';
    bool begin = true;
    for (const auto& [key, value] : dict){
        if (!begin){
            out << ',' << '\n';
        }
        out  << '"'
        << key << '"'
        << ':';
        std::visit(NodePrinter{out}, value.GetNode());
        begin = false;
    }
    out << '}'<< '\n';
}

void NodePrinter::operator()(int integer){
    out << integer;
}

void NodePrinter::operator()(std::string str){
    using namespace std::literals;
    out  << '\"';
    for (auto c = str.begin(); c != str.end(); c++) {
        if (*c == '\"'){
            out << '\\' << '\"';
        } else if (*c == '\n'){
            out << '\\' << 'n';
        } else if (*c == '\r') {
            out << '\\' << 'r';
        } else if (*c == '\t'){
            out << '\\' << 't';
        } else if (*c == '\\'){
            out << '\\';
        } else {
            out << *c;
        }
    }
    out  << '\"';
}
    
void NodePrinter::operator()(bool boolean){
    out << std::boolalpha;
    out << boolean;
}
void NodePrinter::operator()(double duble){
    out << duble;
}

bool operator==(const Node& node1, const Node& node2){
    if (node1.GetNode().index() == node2.GetNode().index()){
        std::ostringstream strm1, strm2;
        std::visit(NodePrinter{strm1}, node1.GetNode());
        std::visit(NodePrinter{strm2}, node2.GetNode());
        return strm1.str() == strm2.str();
    }
    return false;
}

bool operator!=(const Node& node1, const Node& node2){
    return !(node1 == node2);
}

bool operator==(const Document& doc1, const Document& doc2){
    return (doc1.GetRoot() == doc2.GetRoot());
}

bool operator!=(const Document& doc1, const Document& doc2){
    return !(doc1.GetRoot() == doc2.GetRoot());
}

void Print(const Document& doc, std::ostream& output) {
    std::visit(NodePrinter{output}, doc.GetRoot().GetNode());
}

Number LoadNumber(std::istream& input) { //Функция дана по условию задания
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

}  // namespace json
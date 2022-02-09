#include "json_builder.h"

using namespace json;


ValueDictContext DictItemContext::Key(std::string key){
	builder_.Key(std::move(key));
	return ValueDictContext(builder_);
}

Builder& DictItemContext::EndDict(){
	return builder_.EndDict();
}


ValueContext ValueContext::Value(Node value){
	builder_.Value(std::move(value));
	return ValueContext(builder_);
}
ArrayItemContext ValueContext::StartArray(){
	return builder_.StartArray();
}
DictItemContext ValueContext::StartDict(){
	return builder_.StartDict();
}


ArrayItemContext ArrayItemContext::Value(Node value){
	builder_.Value(std::move(value));
	return ArrayItemContext(builder_);
}
ArrayItemContext ArrayItemContext::StartArray(){
	return builder_.StartArray();
}
DictItemContext ArrayItemContext::StartDict(){
	return builder_.StartDict();
}
Builder& ArrayItemContext::EndArray(){
	return builder_.EndArray();
}


DictItemContext ValueDictContext::Value(Node value){
	builder_.Value(std::move(value));
	return DictItemContext(builder_);
}
ArrayItemContext ValueDictContext::StartArray(){
	return builder_.StartArray();
}
DictItemContext ValueDictContext::StartDict(){
	return builder_.StartDict();
}


DictItemContext Builder::StartDict(){
	checkValueExpected("Dict");
	insertDictOrArray(Dict{});
	return DictItemContext(*this);
}

ArrayItemContext Builder::StartArray(){
	checkValueExpected("Array");
	insertDictOrArray(Array{});
	return ArrayItemContext(*this);
}

ValueDictContext Builder::Key(std::string key){
	checkDocumentCompletion("EndArray");
	if (!(nodes_stack_.back()->IsDict())){
		throw std::logic_error("Вызов Key не при открытом словаре");
	}
	if (key_is_last){
		throw std::logic_error("Вызов Key после другого Key");
	}
	last_key_ = key;
	std::get<Dict>(getLastNodeValue()).emplace(std::make_pair(std::move(key), Node{}));
	key_is_last = true;
	return ValueDictContext(*this);
}

Builder& Builder::EndDict(){
	checkDocumentCompletion("EndDict");
	if (nodes_stack_.back()->IsDict()){
		nodes_stack_.pop_back();
	} else {
		throw std::logic_error("Вызов EndDict не при открытом словаре");
	}
	key_is_last = false;
	return *this;
}

Builder& Builder::EndArray(){
	checkDocumentCompletion("EndArray");
	if (nodes_stack_.back()->IsArray()){
		nodes_stack_.pop_back();
	} else {
		throw std::logic_error("Вызов EndArray не при открытом массиве");
	}
	key_is_last = false;
	return *this;
}

Document Builder::Build(){
	if (nodes_stack_.empty()){
		return Document(root_);
	} else {
		throw std::logic_error("Вызов Build на неготовом документе");
	}
}

void Builder::checkValueExpected(std::string node_type){
	checkDocumentCompletion(node_type);
	if (key_is_last || nodes_stack_.back()->IsArray()){
		return ;
	}
	if (nodes_stack_.back()->IsDict() && !key_is_last){
		
		throw std::logic_error("Вызов " + node_type + " вместо Key");
	}
}

void Builder::checkDocumentCompletion(std::string func_name){
	if (nodes_stack_.empty()){
		throw std::logic_error("Вызов " + func_name + " на готовом документе"); 
	}
}

Node::Value& Builder::getLastNodeValue(){
	assert(!nodes_stack_.empty());
	return nodes_stack_.back()->GetValue();
}

const Node::Value& Builder::getLastNodeValue() const{
	assert(!nodes_stack_.empty());
	return nodes_stack_.back()->GetValue();
}

Builder& Builder::Value(Node value){
	checkValueExpected("Value");
	bool key_was_last = key_is_last;
	key_is_last = false;
		if (key_was_last){
			std::get<Dict>(getLastNodeValue()).at(last_key_) = value;
		} else if (nodes_stack_.back()->IsArray()){
			std::get<Array>(getLastNodeValue()).push_back(value);
		} else { // after constructor;
			nodes_stack_.pop_back();
			root_ = value;
	}
	return *this;
}
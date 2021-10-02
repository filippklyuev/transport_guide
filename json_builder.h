#pragma once
#include <cassert>
#include <variant>
#include <vector>
#include <string>
#include <string_view>
#include <utility>

#include "json.h"

namespace json {

class Builder;
class ArrayItemContext;
class ValueContext;
class DictItemContext;
class ValueDictContext;

class BuilderContext {
public:
	BuilderContext(Builder& builder)
		: builder_(builder)
	{}
protected:	
	Builder& builder_;
};

class DictItemContext : protected BuilderContext {
public:
	using BuilderContext::BuilderContext;
	ValueDictContext Key(std::string key);
	Builder& EndDict();
};

class ValueContext : protected BuilderContext {
public:	
	using BuilderContext::BuilderContext;
	ValueContext Value(Node value);
	ArrayItemContext StartArray();
	DictItemContext StartDict();
};

class ArrayItemContext : protected BuilderContext {
public:	
	using BuilderContext::BuilderContext;
	ArrayItemContext Value(Node value);
	ArrayItemContext StartArray();
	DictItemContext  StartDict();
	Builder& EndArray();
};

class ValueDictContext : protected BuilderContext {
public:	
	using BuilderContext::BuilderContext;
	DictItemContext Value(Node value);
	ArrayItemContext StartArray();
	DictItemContext StartDict();	
};

class Builder {
public:
	Builder(){
		nodes_stack_.push_back(&root_);
	}

	Builder(Builder&& other) = delete;

	Builder& operator=(Builder&& other) = delete;

	Builder& operator=(const Builder&) = delete;

	Builder(const Builder&) = delete;

	~Builder(){
		nodes_stack_.clear();
	}

private:
	std::vector<Node*> nodes_stack_;
	Node* dict_value_ptr_ = nullptr;
	Node root_;

	Node::Value& getLastNodeValue();
	const Node::Value& getLastNodeValue() const;
	void checkDocumentCompletion(std::string func_name);

	template<typename Type>
	void insertDictOrArray(Type value){
		if (nodes_stack_.back()->IsDict()){
			*dict_value_ptr_  = value;
			nodes_stack_.push_back(dict_value_ptr_ );
		} else if (nodes_stack_.back()->IsArray()){
			std::get<Array>(getLastNodeValue()).push_back(value);
			nodes_stack_.push_back(&((std::get<Array>(getLastNodeValue())).back()));
		} else { // after constructors
			root_ = value;
		}
	}

public:
	DictItemContext StartDict();
	ValueDictContext Key(std::string key);
	Builder& Value(Node node);
	ArrayItemContext StartArray();
	Builder& EndArray();
	Builder& EndDict();
	Document Build();
};

} // namespace json;


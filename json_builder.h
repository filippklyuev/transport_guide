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
	std::string last_key_;
	Node root_;
	bool key_is_last = false;

	Node::Value& getLastNodeValue();
	const Node::Value& getLastNodeValue() const;
	void checkValueExpected(std::string node_type);
	void checkDocumentCompletion(std::string func_name);

	template<typename Type>
	void insertDictOrArray(Type value){
		if (key_is_last){
			std::get<Dict>(getLastNodeValue()).at(last_key_) = value;
			nodes_stack_.push_back(&(std::get<Dict>(getLastNodeValue())).at(last_key_));
		} else if (nodes_stack_.back()->IsArray()){
			std::get<Array>(getLastNodeValue()).push_back(value);
			nodes_stack_.push_back(&((std::get<Array>(getLastNodeValue())).back()));
		} else { // after constructors
			root_ = value;
		}
		key_is_last = false;
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


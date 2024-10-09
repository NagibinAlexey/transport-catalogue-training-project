#include "json_builder.h"

namespace json {

	Builder::DictKeyContext Builder::Key(std::string string) {

		Builder::DictItemContext dict_item_context(*this);
		Builder::DictKeyContext dict_key_context(dict_item_context, *this);
		if (constructed_) {
			throw std::logic_error("Invalid command");
		}

		Node* key = new Node();
		*key = std::move(string);
		nodes_stack_.emplace_back(key);

		prev_command_ = Command::Key;
		return dict_key_context;
	}

	Builder::DictKeyContext Builder::DictItemContext::Key(std::string string)
	{
		Builder::DictKeyContext dict_key_context(*this, builder_);

		Node* key = new Node();
		*key = std::move(string);
		builder_.nodes_stack_.emplace_back(key);

		builder_.prev_command_ = Command::Key;
		return dict_key_context;
	}

	Builder& Builder::Value(Node value)
	{
		if (constructed_) {
			throw std::logic_error("Invalid command");
		}
		if (nodes_stack_.empty()) {
			if (prev_command_ == Command::Init) {
				constructed_ = true;
			}
			root_ = std::move(value);
		}
		else if (nodes_stack_.back()->IsString()) {
			std::string key = nodes_stack_.back()->AsString();
			nodes_stack_.pop_back();
			Dict& dict = std::get<Dict>(nodes_stack_.back()->GetValue());
			dict[std::move(key)] = std::move(value);
		}
		else if (nodes_stack_.back()->IsArray()) {
			Array& array = std::get<Array>(nodes_stack_.back()->GetValue());
			array.emplace_back(std::move(value));
		}
		prev_command_ = Command::Value;
		return *this;
	}

	Builder::DictItemContext Builder::StartDict()
	{
		Builder::DictItemContext dict_item_context(*this);
		if (constructed_) {
			throw std::logic_error("Invalid command");
		}
		Node* dict = new Node(Dict());
		nodes_stack_.emplace_back(dict);
		prev_command_ = Command::StartDict;
		return dict_item_context;
	}

	Builder::ArrayItemContext Builder::StartArray()
	{
		Builder::ArrayItemContext array_item_context(*this);
		if (constructed_) {
			throw std::logic_error("Invalid command");
		}
		Node* array = new Node(Array());
		nodes_stack_.emplace_back(array);
		prev_command_ = Command::StartArray;
		return array_item_context;
	}

	Builder::DictItemContext Builder::DictKeyContext::Value(Node value)
	{
		Builder::DictKeyContext dict_key_context(*this);
		std::string key = builder_.nodes_stack_.back()->AsString();
		builder_.nodes_stack_.pop_back();
		Dict& dict = std::get<Dict>(builder_.nodes_stack_.back()->GetValue());
		dict[std::move(key)] = std::move(value);
		return dict_key_context.dict_item_context_;
	}

	Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node value)
	{
		Builder::ArrayItemContext array_item_context(*this);
		Array& array = std::get<Array>(builder_.nodes_stack_.back()->GetValue());
		array.emplace_back(std::move(value));

		builder_.prev_command_ = Command::Value;
		return array_item_context;
	}

	Builder& Builder::EndDict()
	{
		Dict& dict = std::get<Dict>(nodes_stack_.back()->GetValue());
		nodes_stack_.pop_back();
		prev_command_ = Command::EndDict;
		Builder::Value(std::move(dict));
		return *this;
	}

	Builder& Builder::EndArray()
	{
		if (constructed_) {
			throw std::logic_error("Invalid command");
		}
		Array& array = std::get<Array>(nodes_stack_.back()->GetValue());
		nodes_stack_.pop_back();
		prev_command_ = Command::EndArray;
		Builder::Value(std::move(array));
		return *this;
	}

	Node Builder::Build()
	{
		if (prev_command_ == Command::Init) {
			throw std::logic_error("Node must be not empty");
		}
		else if (nodes_stack_.size() != 0) {
			throw std::logic_error("Not closed dict or array");
		}
		return root_;
	}
}

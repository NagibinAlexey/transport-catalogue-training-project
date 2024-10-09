#pragma once

#include "json.h"
#include <stdexcept>
#include <string>

namespace json {

	enum class Command {
		Init,
		Key,
		Value,
		StartDict,
		StartArray,
		EndDict,
		EndArray,
	};

	class Builder {
	private:
		class DictItemContext;
		class ArrayItemContext;
		class DictKeyContext;
	public:
		Builder& Value(Node value);
		DictKeyContext Key(std::string string);
		DictItemContext StartDict();
		Builder& EndDict();
		ArrayItemContext StartArray();
		Builder& EndArray();
		Node Build();

	private:
		Node root_;
		std::vector<Node*> nodes_stack_;
		Command prev_command_ = Command::Init;
		bool constructed_ = false;

		class DictItemContext {
		public:
			DictItemContext(Builder& builder) : builder_(builder) {}
			DictKeyContext Key(std::string string);
			Builder& EndDict() {
				return builder_.EndDict();
			}
		private:
			Builder& builder_;
		};

		class ArrayItemContext {
		public:
			ArrayItemContext(Builder& builder) : builder_(builder) {}
			ArrayItemContext Value(Node value);
			ArrayItemContext StartArray() {
				return builder_.StartArray();
			}
			Builder& EndArray() {
				return builder_.EndArray();
			}
			DictItemContext StartDict() {
				return builder_.StartDict();
			}
		private:
			Builder& builder_;
		};

		class DictKeyContext {
		public:
			DictKeyContext(DictItemContext dict_item_context, Builder& builder) : dict_item_context_(dict_item_context), builder_(builder) {}
			DictItemContext Value(Node value);
			ArrayItemContext StartArray() {
				return builder_.StartArray();
			}
			DictItemContext StartDict() {
				return builder_.StartDict();
			}
		private:
			DictItemContext dict_item_context_;
			Builder& builder_;
		};
	};
}
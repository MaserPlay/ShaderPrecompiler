#pragma once

#include <string>
#include <type_traits>
#include <set>

#include "lexer.hpp"

namespace shader_precompiler::ast {

	namespace nodes {

		inline std::string ident(const std::size_t nesting) {
			return std::string(nesting, '\t');
		}
		struct Node {
			virtual ~Node() = default;
			virtual std::string toDebugString(std::size_t nesting) const = 0;
			virtual bool equals(const Node& other) const = 0;
			template<class N, class E>
			inline static bool nodeEq(const std::unique_ptr<N>& a,
				const std::unique_ptr<E>& b)
			{
				static_assert(std::is_base_of_v<Node, E>);
				static_assert(std::is_base_of_v<Node, N>);
				if (!a || !b)
					return a == b;

				return a->equals(*b);
			}

			inline bool operator==(const Node& b) {
				return this->equals(b);
			}
		};

		struct CodeBlock : Node {
			std::vector<std::unique_ptr<Node>> expressions;
			std::string toDebugString(std::size_t nesting) const override {
				std::string final{};
				final += ident(nesting) + "CodeBlock:\n";
				for (auto& e : expressions)
				{
					if (e) {
						final += e->toDebugString(nesting + 1) + '\n';
					}
				}
				if (expressions.empty()) {
					final += ident(nesting + 1) + "EMPTY";
				}
				return final;
			}
			bool equals(const Node& other) const override {
				auto p = dynamic_cast<const CodeBlock*>(&other);
				if (!p) return false;

				if (expressions.size() != p->expressions.size())
					return false;

				for (size_t i = 0; i < expressions.size(); ++i) {
					if (!nodeEq(expressions[i], p->expressions[i]))
						return false;
				}

				return true;
			}
		};

		struct Identifier : Node {
			std::string name;
			Identifier() = default;
			Identifier(std::string name) : name(name) {}
			std::string toDebugString(std::size_t nesting) const override {
				return ident(nesting) + name;
			}
			bool equals(const Node& other) const override {
				if (auto p = dynamic_cast<const Identifier*>(&other))
					return name == p->name;
				return false;
			}
		};

		struct VariableInitialization : Node {
			std::unique_ptr<Identifier> type;
			std::unique_ptr<Identifier> name;
			VariableInitialization() = default;
			VariableInitialization(std::unique_ptr<Identifier> type, std::unique_ptr<Identifier> name) : type(std::move(type)), name(std::move(name)) {}
			std::string toDebugString(std::size_t nesting) const override {
				std::string final = ident(nesting) + "VariableInitialization:\n";
				if (type) {
					final += type->toDebugString(nesting + 1) + '\n';
				}
				else {
					final += ident(nesting + 1) + "NULL\n";
				}
				if (name) {
					final += name->toDebugString(nesting + 1);
				}
				else {
					final += ident(nesting + 1) + "NULL";
				}
				return final;
			}
			bool equals(const Node& other) const override {
				if (auto p = dynamic_cast<const VariableInitialization*>(&other))
				{
					return nodeEq(p->name, this->name) && nodeEq(p->type, this->type);
				}
				return false;
			}
		};

		struct Operator : Node {
			std::unique_ptr<Node> left;
			std::string op;
			std::unique_ptr<Node> right;
			std::string toDebugString(std::size_t nesting) const override {
				std::string final = ident(nesting) + "Operator:\n";

				if (left) {
					final += left->toDebugString(nesting + 1) + '\n';
				}
				else {
					final += ident(nesting + 1) + "NULL\n";
				}

				final += ident(nesting + 1) + op + '\n';

				if (right) {
					final += right->toDebugString(nesting + 1);
				}
				else {
					final += ident(nesting + 1) + "NULL";
				}

				return final;
			}
			Operator() = default;
			Operator(std::unique_ptr<Node> left, std::string op, std::unique_ptr<Node> right) : 
				left(std::move(left)), right(std::move(right)), op(op) {}

			bool equals(const Node& other) const override {
				auto p = dynamic_cast<const Operator*>(&other);
				if (!p) return false;

				return op == p->op &&
					nodeEq(left, p->left) &&
					nodeEq(right, p->right);
			}
		};

		struct Func : Node {
			std::unique_ptr<CodeBlock> code;
			std::unique_ptr<Identifier> name;
			std::unique_ptr<Identifier> returnType;

			std::string toDebugString(std::size_t nesting) const override {
				std::string final = ident(nesting) + "Func:\n";
				final += name->toDebugString(nesting + 1) + '\n';
				final += code->toDebugString(nesting + 1);
				return final;
			}

			bool equals(const Node& other) const override {
				auto p = dynamic_cast<const Func*>(&other);
				if (!p) return false;

				return nodeEq(code, p->code) &&
					nodeEq(name, p->name) &&
					nodeEq(returnType, p->returnType);
			}
			Func(std::unique_ptr<CodeBlock> code, std::unique_ptr<Identifier> name, std::unique_ptr<Identifier> returnType) : 
				returnType(std::move(returnType)), name(std::move(name)), code(std::move(code)) {}
		};

		struct NumberExpr : Node {
			std::size_t value;
			NumberExpr(std::size_t name) : value(name) {}
			std::string toDebugString(std::size_t nesting) const override {
				return ident(nesting) + std::to_string(value);
			}

			bool equals(const Node& other) const override {
				if (auto p = dynamic_cast<const NumberExpr*>(&other))
					return value == p->value;
				return false;
			}
		};
	};

	class AstParser {
		shader_precompiler::lexer::BaseLexerStream& from;
		std::shared_ptr<nodes::CodeBlock> base;
		std::set<std::string> types{ "vec3", "int", "double" };
	public:
		AstParser(shader_precompiler::lexer::BaseLexerStream& stream) : from(stream) {}

		std::unique_ptr<nodes::Node> parseExpression(int minPrec = 0);
		std::unique_ptr<nodes::Node> parseSingle(bool onlyPeek = false);
		std::unique_ptr<nodes::Node> parseInitialization();
		std::shared_ptr<nodes::CodeBlock> createTree();
	};
}
#pragma once

#include <string>
#include <type_traits>
#include <set>

#include "lexer.hpp"
#include "shader_precompiler.hpp"

namespace shader_precompiler::ast {

	namespace nodes {
		class CodeBlock;
		class Identifier;
		class Return;
		class Attribute;
		class VariableInitialization;
		class IfElse;
		class Operator;
		class FuncDeclaration;
		class FuncCall;
		class Func;
		class NumberExpr;
	};

	class VisitorBase {
	public:
		virtual void visit(nodes::CodeBlock& node) = 0;
		virtual void visit(nodes::Identifier& node) = 0;
		virtual void visit(nodes::Return& node) = 0;
		virtual void visit(nodes::Attribute& node) = 0;
		virtual void visit(nodes::VariableInitialization& node) = 0;
		virtual void visit(nodes::IfElse& node) = 0;
		virtual void visit(nodes::Operator& node) = 0;
		virtual void visit(nodes::FuncDeclaration& node) = 0;
		virtual void visit(nodes::FuncCall& node) = 0;
		virtual void visit(nodes::Func& node) = 0;
		virtual void visit(nodes::NumberExpr& node) = 0;
	};

	namespace nodes {
		struct Node {
			virtual ~Node() = default;
			virtual std::string toDebugString(std::size_t nesting) const = 0;
			template<class N>
			inline bool operator==(const N& b) const {
				static_assert(std::is_base_of_v<Node, N>);
				return this->equals(b);
			}
			template<class N>
			inline bool operator!=(const N& b) const {
				static_assert(std::is_base_of_v<Node, N>);
				return !this->equals(b);
			}
			virtual void accept(VisitorBase& base) = 0;
		protected:
			virtual bool equals(const Node& other) const = 0;
			inline static std::string ident(const std::size_t nesting) {
				return std::string(nesting, '\t');
			}

			template<class N, class E>
			inline static bool nodeEq(const std::unique_ptr<N>& a,
				const std::unique_ptr<E>& b)
			{
				static_assert(std::is_base_of_v<Node, E>);
				static_assert(std::is_base_of_v<Node, N>);
				if (!a || !b)
					return a == b;

				return *a == *b;
			}
			template<class T>
			inline static std::string unique_ptr_to_debug_string(const std::unique_ptr<T>& input, const std::size_t nesting) {
				if (input) {
					return input->toDebugString(nesting);
				}
				else {
					return ident(nesting) + "NULL";
				}
			}

			template<class T>
			inline static bool vectorEquals(const std::vector<std::unique_ptr<T>>& a,
				const std::vector<std::unique_ptr<T>>& b) {
				static_assert(std::is_base_of_v<Node, T>);

				if (a.size() != b.size())
					return false;

				for (size_t i = 0; i < a.size(); ++i) {
					if (!nodeEq(a[i], b[i]))
						return false;
				}
				return true;
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
			CodeBlock() = default;
			CodeBlock(std::vector<std::unique_ptr<Node>> expressions) : expressions(std::move(expressions)) {};

			void accept(VisitorBase& base) override {
				base.visit(*this);
			}
		protected:
			bool equals(const Node& other) const override {
				auto p = dynamic_cast<const CodeBlock*>(&other);
				if (!p) return false;

				return vectorEquals(expressions, p->expressions);
			}
		};

		struct Identifier : Node {
			std::string name;
			Identifier() = default;
			Identifier(std::string name) : name(name) {}
			std::string toDebugString(std::size_t nesting) const override {
				return ident(nesting) + name;
			}
			void accept(VisitorBase& base) override {
				base.visit(*this);
			}
		protected:
			bool equals(const Node& other) const override {
				if (auto p = dynamic_cast<const Identifier*>(&other))
					return name == p->name;
				return false;
			}
		};

		struct Return : Node {
			std::unique_ptr<Node> value;
			Return() = default;
			Return(std::unique_ptr<Node> value) : value(std::move(value)) {}
			std::string toDebugString(std::size_t nesting) const override {
				std::string final = ident(nesting) + "Return:\n";
				final += unique_ptr_to_debug_string(value, nesting + 1) + '\n';
				return final;
			}
			void accept(VisitorBase& base) override {
				base.visit(*this);
			}
		protected:
			bool equals(const Node& other) const override {
				if (auto p = dynamic_cast<const Return*>(&other))
				{
					return nodeEq(p->value, this->value);
				}
				return false;
			}
		};

		struct Attribute : Node {
			std::unique_ptr<Node> value;
			Attribute() = default;
			Attribute(std::unique_ptr<Node> value) : value(std::move(value)) {}
			std::string toDebugString(std::size_t nesting) const override {
				std::string final = ident(nesting) + "Attribute:\n";
				final += unique_ptr_to_debug_string(value, nesting + 1) + '\n';
				return final;
			}
			void accept(VisitorBase& base) override {
				base.visit(*this);
			}
		protected:
			bool equals(const Node& other) const override {
				if (auto p = dynamic_cast<const Attribute*>(&other))
				{
					return nodeEq(p->value, this->value);
				}
				return false;
			}
		};

		struct VariableInitialization : Node {
			std::unique_ptr<Identifier> type;
			std::unique_ptr<Identifier> name;
			std::vector<std::unique_ptr<Attribute>> attributes{};
			VariableInitialization() = default;
			VariableInitialization(std::unique_ptr<Identifier> type, std::unique_ptr<Identifier> name, std::vector<std::unique_ptr<Attribute>> attributes) : type(std::move(type)), name(std::move(name)), attributes(std::move(attributes)) {}
			VariableInitialization(std::unique_ptr<Identifier> type, std::unique_ptr<Identifier> name) : type(std::move(type)), name(std::move(name)) {}
			std::string toDebugString(std::size_t nesting) const override {
				std::string final = ident(nesting) + "VariableInitialization:\n";

				if (attributes.empty()) {
					final += ident(nesting + 1) + "ATTRIBUTES_EMPTY\n";
				}
				else {
					final += ident(nesting + 1) + "ATTRIBUTES:\n";
					for (auto& e : attributes)
					{
						if (e) {
							final += unique_ptr_to_debug_string(e, nesting + 2) + '\n';
						}
					}
				}
				final += unique_ptr_to_debug_string(type, nesting + 1) + '\n';
				final += unique_ptr_to_debug_string(name, nesting + 1);
				return final;
			}
			void accept(VisitorBase& base) override {
				base.visit(*this);
			}
		protected:
			bool equals(const Node& other) const override {
				if (auto p = dynamic_cast<const VariableInitialization*>(&other))
				{
					return nodeEq(p->name, this->name) && nodeEq(p->type, this->type) &&
						vectorEquals(attributes, p->attributes);
				}
				return false;
			}
		};

		struct IfElse : Node {
			std::unique_ptr<Node> ifCondition;
			std::unique_ptr<Node> thenBranch;
			std::unique_ptr<Node> elseBranch;

			std::string toDebugString(std::size_t nesting) const override {
				std::string final = ident(nesting) + "IfElse:\n";

				final += unique_ptr_to_debug_string(ifCondition, nesting + 1) + '\n';
				final += unique_ptr_to_debug_string(thenBranch, nesting + 1) + '\n';
				final += unique_ptr_to_debug_string(elseBranch, nesting + 1);

				return final;
			}
			IfElse() = default;
			IfElse(std::unique_ptr<Node> ifCondition, std::unique_ptr<Node> thenBranch, std::unique_ptr<Node> elseBranch) : ifCondition(std::move(ifCondition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}

			void accept(VisitorBase& base) override {
				base.visit(*this);
			}
		protected:
			bool equals(const Node& other) const override {
				auto p = dynamic_cast<const IfElse*>(&other);
				if (!p) return false;

				return nodeEq(ifCondition, p->ifCondition) &&
					nodeEq(thenBranch, p->thenBranch) &&
					nodeEq(elseBranch, p->elseBranch);
			}
		};

		struct Operator : Node {
			std::unique_ptr<Node> left;
			std::string op;
			std::unique_ptr<Node> right;
			std::string toDebugString(std::size_t nesting) const override {
				std::string final = ident(nesting) + "Operator:\n";

				final += unique_ptr_to_debug_string(left, nesting + 1) + '\n';

				final += ident(nesting + 1) + op + '\n';

				final += unique_ptr_to_debug_string(right, nesting + 1);

				return final;
			}
			Operator() = default;
			Operator(std::unique_ptr<Node> left, std::string op, std::unique_ptr<Node> right) : 
				left(std::move(left)), right(std::move(right)), op(op) {}

			void accept(VisitorBase& base) override {
				base.visit(*this);
			}
		protected:
			bool equals(const Node& other) const override {
				auto p = dynamic_cast<const Operator*>(&other);
				if (!p) return false;

				return op == p->op &&
					nodeEq(left, p->left) &&
					nodeEq(right, p->right);
			}
		};

		struct FuncDeclaration : Node {
			std::unique_ptr<Identifier> name;
			std::unique_ptr<Identifier> returnType;
			std::vector<std::unique_ptr<VariableInitialization>> params;
			std::vector<std::unique_ptr<Attribute>> attributes{};

			std::string toDebugString(std::size_t nesting) const override {
				std::string final = ident(nesting) + "FuncDeclaration:\n";

				if (attributes.empty()) {
					final += ident(nesting + 1) + "ATTRIBUTES_EMPTY\n";
				}
				else {
					final += ident(nesting + 1) + "ATTRIBUTES:\n";
					for (auto& e : attributes)
					{
						if (e) {
							final += unique_ptr_to_debug_string(e, nesting + 2) + '\n';
						}
					}
				}

				final += unique_ptr_to_debug_string(name, nesting + 1) + '\n';

				if (params.empty()) {
					final += ident(nesting + 1) + "PARAMS_EMPTY\n";
				}
				else {
					final += ident(nesting + 1) + "PARAMS:\n";
					for (auto& e : params)
					{
						if (e) {
							final += unique_ptr_to_debug_string(e, nesting + 2) + '\n';
						}
					}
				}

				return final;
			}

			FuncDeclaration() = default;
			FuncDeclaration(std::unique_ptr<Identifier> returnType, std::unique_ptr<Identifier> name, std::vector<std::unique_ptr< VariableInitialization>> params, std::vector<std::unique_ptr<Attribute>> attributes) :
				returnType(std::move(returnType)), name(std::move(name)), params(std::move(params)), attributes(std::move(attributes)) {}
			FuncDeclaration(std::unique_ptr<Identifier> returnType, std::unique_ptr<Identifier> name, std::vector<std::unique_ptr< VariableInitialization>> params) :
				returnType(std::move(returnType)), name(std::move(name)), params(std::move(params)) {}

			void accept(VisitorBase& base) override {
				base.visit(*this);
			}
		protected:
			bool equals(const Node& other) const override {
				auto p = dynamic_cast<const FuncDeclaration*>(&other);
				if (!p) return false;

				return nodeEq(name, p->name) &&
					nodeEq(returnType, p->returnType) &&
					vectorEquals(params, p->params) &&
					vectorEquals(attributes, p->attributes);
			}
		};

		struct FuncCall : Node {
			std::unique_ptr<Identifier> name;
			std::vector<std::unique_ptr< Node>> params;

			std::string toDebugString(std::size_t nesting) const override {
				std::string final = ident(nesting) + "FuncCall:\n";
				final += unique_ptr_to_debug_string(name, nesting + 1) + '\n';

				final += ident(nesting + 1) + "PARAMS:\n";
				for (auto& e : params)
				{
					if (e) {
						final += unique_ptr_to_debug_string(e, nesting + 2) + '\n';
					}
				}
				if (params.empty()) {
					final += ident(nesting + 1) + "PARAMS_EMPTY\n";
				}

				return final;
			}

			FuncCall() = default;
			FuncCall(std::unique_ptr<Identifier> name, std::vector<std::unique_ptr< Node>> params) :
				name(std::move(name)), params(std::move(params)) {}
			void accept(VisitorBase& base) override {
				base.visit(*this);
			}
		protected:
			bool equals(const Node& other) const override {
				auto p = dynamic_cast<const FuncCall*>(&other);
				if (!p) return false;

				return nodeEq(name, p->name) &&
					vectorEquals(params, p->params);
			}
		};

		struct Func : Node {
			std::unique_ptr<FuncDeclaration> declaration;
			std::unique_ptr<CodeBlock> code;

			std::string toDebugString(std::size_t nesting) const override {
				std::string final = ident(nesting) + "Func:\n";
				final += unique_ptr_to_debug_string(declaration, nesting + 1);
				final += unique_ptr_to_debug_string(code, nesting + 1);
				return final;
			}

			Func() = default;
			Func(std::unique_ptr<FuncDeclaration> declaration, std::unique_ptr<CodeBlock> code) :
				code(std::move(code)), declaration(std::move(declaration)) {}
			void accept(VisitorBase& base) override {
				base.visit(*this);
			}
		protected:
			bool equals(const Node& other) const override {
				auto p = dynamic_cast<const Func*>(&other);
				if (!p) return false;

				return nodeEq(code, p->code) &&
					nodeEq(declaration, p->declaration);
			}
		};

		struct NumberExpr : Node {
			std::size_t value;
			NumberExpr(std::size_t name) : value(name) {}
			std::string toDebugString(std::size_t nesting) const override {
				return ident(nesting) + std::to_string(value);
			}
			void accept(VisitorBase& base) override {
				base.visit(*this);
			}

		protected:
			bool equals(const Node& other) const override {
				if (auto p = dynamic_cast<const NumberExpr*>(&other))
					return value == p->value;
				return false;
			}
		};
	};

	class BaseAstProcessor{
	public:
		virtual std::shared_ptr<nodes::CodeBlock> processTree() = 0;
	};

	class AstParser : public BaseAstProcessor {
		shader_precompiler::lexer::BaseLexerStream& from;
		std::shared_ptr<nodes::CodeBlock> base;
		std::set<std::string> types{ "vec3", "int", "double", "void" };

		std::unique_ptr<nodes::Node> parseExpression(std::unique_ptr<shader_precompiler::ast::nodes::Node> left, int minPrec = 0);
		std::unique_ptr<nodes::Node> parseSingle();
		std::unique_ptr<nodes::Return> parseReturn();
		std::unique_ptr<nodes::Node> parsePrimary();
		std::unique_ptr<nodes::Node> parseIfElse();
		std::unique_ptr<nodes::Attribute> parseAttributes();
		std::unique_ptr<nodes::Node> parseFunctionCall(std::unique_ptr<nodes::Identifier> name);
		std::unique_ptr<nodes::VariableInitialization> parseVariableInitialization(std::unique_ptr<shader_precompiler::ast::nodes::Node> first, std::unique_ptr<shader_precompiler::ast::nodes::Node> second, std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Attribute>> attributes = {});
		std::unique_ptr<nodes::Node> parseFunction(std::unique_ptr<shader_precompiler::ast::nodes::Node> first, std::unique_ptr<shader_precompiler::ast::nodes::Node> second, std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Attribute>> attributes = {});
		std::unique_ptr<nodes::Node> parseDeclaration(std::vector<std::unique_ptr<shader_precompiler::ast::nodes::Attribute>> attributes = {});
		std::unique_ptr<nodes::Node> parseBrackets();
		std::unique_ptr<nodes::CodeBlock> parseCodeBlock();

		IDiagnosticReporter& reporter;

		PRINT_ERROR_DEFINE(shader_precompiler::Error::Stage::AST)

		inline bool isType(std::string s) {
			return types.find(s) != end(types);
		}
	public:
		AstParser(shader_precompiler::lexer::BaseLexerStream& stream, IDiagnosticReporter& reporter) : from(stream), reporter(reporter) {}
		std::shared_ptr<nodes::CodeBlock> processTree() override;
	};
}
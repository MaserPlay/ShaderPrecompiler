#pragma once

#include <string>
#include <functional>

#include <fmt/args.h>

namespace shader_precompiler {

	enum class ShaderLanguages {
		GLSL,
		ESSL
	};

	struct Location {
		std::size_t line{};
		std::size_t column{};
		Location() = default;
		Location(std::size_t line, std::size_t column) : line(line), column(column) {}
		std::string toString() const
		{
			return std::to_string(line) + ":" + std::to_string(column);
		}

		operator std::string() const
		{
			return toString();
		}
	};


	struct Error
	{
		enum class ErrorCodes {
			// LEXER
			UNEXPECTED_CHAR = 1000,

			//PRECOMPILER
			DELETING_BY_DEFINE = 2000,
			LEVEL_DIRECTIVE,
			UNEXPECTED_DIRECTIVE,
			NO_DIRECTIVE_NAME,
			UNEXPECTED_MACRO_NAME,
			UNEXPECTED_TOKEN,

			//AST
			UNEXPECTED_START_TOKEN = 3000,
			NO_CLOSE_ATTRIBUTE_TOKEN,
			NO_CLOSE_BRACKET_TOKEN,
			TYPE_ALONE,

			//SEMANTIC
			UNDEFINDED_TYPE = 4000,
			UNDEFINDED_VARIABLE,
			UNDEFINDED_FUNCTION,
			TOKEN_IS_NULL,
			REDEFINITION_VARIABLE,
			REDEFINITION_FUNCTION,

			//GLSL
			MULTIPLE_ATTRIBUTES = 5000,
			LAYOUT_IN_INVALID_PARAMS,
		};

		enum class Stage
		{
			PREPROCESSOR,
			LEXER,
			AST,
			MINIMAZER,
			SEMANTIC_AMALIZITER,
			GLSL
		};
		enum class Level {
			INFO,
			WARNING,
			ERROR,
			FATAL
		};
		static char stageToLetter(const Stage& stage) {
			switch (stage) {
				case Stage::PREPROCESSOR: 
					return 'P';
				case Stage::LEXER:
					return 'L';
				case Stage::AST:
					return 'A';
				case Stage::SEMANTIC_AMALIZITER:
					return 'SA';
				case Stage::MINIMAZER:
					return 'M';
				default:
					return 'NOT';
			}
		}
		static char levelToLetter(const Level& stage) {
			switch (stage) {
			case Level::INFO:
				return 'I';
			case Level::WARNING:
				return 'W';
			case Level::ERROR:
				return 'E';
			case Level::FATAL:
				return 'F';
			default:
				return 'NOT';
			}
		}
		static std::string levelToString(const Level& stage) {
			switch (stage) {
			case Level::INFO:
				return "INFO";
			case Level::WARNING:
				return "WARNING";
			case Level::ERROR:
				return "ERROR";
			case Level::FATAL:
				return "FATAL";
			default:
				return "NOT";
			}
		}
		Stage stage{};
		Level level{};
		ErrorCodes code{};

		::fmt::dynamic_format_arg_store<fmt::format_context> args{};

		template<typename... Args>
		static inline auto makeStore(Args&&... args) {
			fmt::dynamic_format_arg_store<fmt::format_context> store;
			(store.push_back(args), ...);
			return store;
		}

		shader_precompiler::Location location{};

		Error() = default;

		Error(Level level, Stage stage, ErrorCodes code, ::fmt::dynamic_format_arg_store<fmt::format_context> args, shader_precompiler::Location location) :
			code(code), stage(stage), args(std::move(args)), level(level), location(location) {};

		Error(Level level, Stage stage, ErrorCodes code, ::fmt::dynamic_format_arg_store<fmt::format_context> args, std::size_t line, std::size_t column) :
			Error(level, stage, code, std::move(args), Location(line, column)) {};

#define PRINT_ERROR_DEFINE(STAGE) \
		inline void printError(shader_precompiler::Error::Level level, shader_precompiler::Error::ErrorCodes code, ::fmt::dynamic_format_arg_store<fmt::format_context> args, shader_precompiler::Location location) { \
			reporter.report(shader_precompiler::Error{ \
				level, STAGE, code, std::move(args), location }); \
		} \
		inline void printError(shader_precompiler::Error::Level level, shader_precompiler::Error::ErrorCodes code, ::fmt::dynamic_format_arg_store<fmt::format_context> args, shader_precompiler::lexer::Token token) { \
			printError( \
				level, code, std::move(args), token.location \
			); \
		}
	};

	class IDiagnosticReporter {
	public:
		virtual ~IDiagnosticReporter() = default;

		virtual void report(const Error& error) = 0;
	};
};
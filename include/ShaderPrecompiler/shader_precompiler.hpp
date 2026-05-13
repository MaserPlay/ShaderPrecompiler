#pragma once

#include <string>
#include <functional>

namespace shader_precompiler {

	enum class ShaderLanguages {
		GLSL,
		ESSL
	};


	struct Error
	{
		enum class Stage
		{
			LEXER,
			PREPROCESSOR,
			AST,
		};
		static char stageToLetter(const Stage& stage) {
			switch (stage) {
				case Stage::PREPROCESSOR: 
					return 'P';
				case Stage::LEXER:
					return 'L';
				case Stage::AST:
					return 'A';
				default:
					return 'NOT';
			}
		}
		Stage stage{};
		std::size_t code{};
		std::string text{};

		std::size_t line{};
		std::size_t column{};
		Error() = default;
		Error(Stage stage, std::size_t code, std::string text, std::size_t line, std::size_t column) : 
			column(column), line(line), code(code), stage(stage), text(text) {};
	};

	void setErrorOutput(std::function<void(const Error& error)>);
};
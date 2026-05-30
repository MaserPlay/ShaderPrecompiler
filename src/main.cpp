#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <argparse/argparse.hpp>

#include "shader_precompiler.hpp"
#include "lexer.hpp"
#include "precompiler.hpp"
#include "diagnostic_reporters.hpp"
#include "messages.hpp"
#include "minimazer.hpp"
#include "semantic.hpp"
#include "to_glsl.hpp"

#define SHADER_LANGUAGES_VALUES_STRINGS "GLSL", "ESSL"

void createArgumentApi(argparse::ArgumentParser& program) {
	auto& input = program.add_mutually_exclusive_group(true);
	input.add_argument("--input_file", "-if").help("Input by reading file");
	input.add_argument("--code", "-c").help("Input by input arg");
	input.add_argument("--std_cin", "-stdin").help("Input by std::cin").flag();

	//std::string shader_langs[]{ SHADER_LANGUAGES_VALUES_STRINGS };

	//std::string list;
	//for (const auto& s : shader_langs)
	//{
	//	list += ", " + s;
	//}

	//program.add_argument("--out_language", "-ol")
	//	.help("what shader language to use to output the result. One of" + list)
	//	.required()
	//	.choices(SHADER_LANGUAGES_VALUES_STRINGS);

	auto& output = program.add_mutually_exclusive_group();
	output.add_argument("--output_file", "-of").help("Output by writing file");
	output.add_argument("--std_cout", "-stdout").help("Output by std::cout").flag();
}

void collectInputCode(const argparse::ArgumentParser& program, std::function<void(std::istream&)> workWithStream) {

	// Collect Input Code
	if (auto file_name = program.present<std::string>("--input_file")) {
		std::ifstream file(*file_name);

		if (!file.is_open()) {
			std::cerr << "Failed to open file: " << *file_name << std::endl;
			std::exit(EXIT_FAILURE);
		}
		workWithStream(file);

	}
	else if (program.get<bool>("--std_cin") == true) {
		workWithStream(std::cin);
	}
	else if (auto code_arg = program.present<std::string>("--code")) {
		std::istringstream str(*code_arg);
		workWithStream(str);
	}
	else {
		std::cerr << "Failed to get input_code" << std::endl;
		std::abort();
	}

}

static void processAll(std::istream& in, std::ostream& out) {
	shader_precompiler::PrintDiagnostic prDa(shader_precompiler::locales::Locales::ENGLISH, std::cerr);

	shader_precompiler::CalcDiagnostic calcDa(prDa);

	shader_precompiler::lexer::LexerStream tokenStream(in, calcDa);

	shader_precompiler::precompiler::PrecompilerLexerStream afterPreprocessor(tokenStream, calcDa);

	shader_precompiler::ast::AstParser ast(afterPreprocessor, calcDa);

	shader_precompiler::visitors::MinimazerVisitor min(ast, da);

	shader_precompiler::SemanticVisitor sem(min, calcDa);

	shader_precompiler::GlslVisitor glsl(sem, calcDa, out);

	glsl.generate();
}

shader_precompiler::ShaderLanguages getShaderLanguage(const argparse::ArgumentParser& program) {
	shader_precompiler::ShaderLanguages shl;
	std::string arg;
	try {
		arg = program.get<std::string>("--out_language");
	}
	catch (std::bad_any_cast e) {
		std::cerr << "-ol doesn`t set" << std::endl;
		std::cout << program; // printing help message
		std::exit(EXIT_FAILURE);
	}

	std::string shader_langs[]{ SHADER_LANGUAGES_VALUES_STRINGS };
	auto result = std::find(begin(shader_langs), end(shader_langs), arg);

	if (result == end(shader_langs))
	{
		std::cerr << "Value not found" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	else
		shl = (shader_precompiler::ShaderLanguages)(result - begin(shader_langs));

	return shl;
}

void outputResult(const argparse::ArgumentParser& program, std::function<void(std::ostream&)> workWithStream) {
	if (program.get<bool>("--std_cout") == true) {
		workWithStream(std::cout);
	}
	else {
		std::string file_name = program.present<std::string>("--output_file")
			.value_or("out" + program.present<std::string>("--input_file")
				.value_or(".txt")
			);

		std::ofstream out;          // поток для записи
		out.open(file_name);      // открываем файл для записи

		if (out.is_open())
		{
			workWithStream(out);
		}

		out.close();
	}
}


int main(int argc, char* argv[]) {
	argparse::ArgumentParser program{ APPNAME, APPVERSION };
	createArgumentApi(program);

	try {
		program.parse_args(argc, argv);
	}
	catch (const std::exception& err) {
		std::cerr << err.what() << std::endl;
		std::cout << program; // printing help message
		return EXIT_FAILURE;
	}

	//shader_precompiler::ShaderLanguages shl = getShaderLanguage(program);

	collectInputCode(program, [&program](std::istream& in) {
		outputResult(program, [&in](std::ostream& out) {processAll(in, out); }); });

	return 0;
}
#include "precompiler.hpp"

#include <iostream>

#include "shader_precompiler.hpp"


namespace shader_precompiler {
	void process(std::istream& code_, std::ostringstream& out, const shader_precompiler::ShaderLanguages language) {
		precompiler::process(code_, out);
	}


	std::function<void(const shader_precompiler::Error& error)> errorFunc =
		[](const shader_precompiler::Error& error) {
		std::cout 
			<< "ERROR: " << shader_precompiler::Error::stageToLetter(error.stage) << error.code << "   " 
			<< error.text << "   " 
			<< error.line << ':' << error.column 
			<< std::endl;
		};

	void setError(shader_precompiler::Error error) {
		errorFunc(error);
	}
	void setErrorOutput(std::function<void(const shader_precompiler::Error& error)> func) {
		errorFunc = func;
	}
};
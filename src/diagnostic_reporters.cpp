#include "diagnostic_reporters.hpp"

#include <iostream>

void shader_precompiler::PrintDiagnostic::report(const shader_precompiler::Error& error) {
	out
		<< shader_precompiler::Error::levelToString(error.level).c_str() << ": " << (std::size_t)error.code << "   "
		<< fmt::vformat(shader_precompiler::locales::getString(locale, error.code), error.args).c_str() << "   "
		<< error.location.line << ':' << error.location.column
		<< std::endl;
	hasErrors_ = true;
}
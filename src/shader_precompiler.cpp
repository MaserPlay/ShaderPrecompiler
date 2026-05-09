#include "precompiler/precompiler.hpp"
#include "shader_precompiler.hpp"


namespace shader_precompiler {
	std::string process(const std::string code_) {
		std::string code = code_;
		code = precompiler::process(code);

		return code;
	}
};
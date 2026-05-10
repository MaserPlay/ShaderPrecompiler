#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>

namespace precompiler {
	std::string process(std::istream& code, const std::map<std::string, std::string>& startupDefines = {});
	inline std::string process(const std::string code, const std::map<std::string, std::string>& startupDefines = {}) {
		std::istringstream iss(code);   // создаём поток из строки
		return process(iss, startupDefines);
	}
};
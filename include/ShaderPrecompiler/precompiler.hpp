#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>

namespace precompiler {
	void process(std::istream& input, std::ostringstream& output, const std::map<std::string, std::string>& startupDefines = {});
	inline std::string process(const std::string code, const std::map<std::string, std::string>& startupDefines = {}) {
		std::istringstream iss(code);   // создаём поток из строки
		std::ostringstream oss{};
		process(iss, oss, startupDefines);
		return oss.str();
	}
};
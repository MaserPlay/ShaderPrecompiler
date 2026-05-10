#pragma once

#include <string>
#include <algorithm>
#include <cctype>

namespace string_utils {
	std::string::size_type findIgnoreCaps(const std::string base, const std::string findStr, const std::size_t pos = 0);
};
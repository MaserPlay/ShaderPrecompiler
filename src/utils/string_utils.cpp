#include "string_utils.hpp"

namespace string_utils {
	std::string::size_type findIgnoreCaps(const std::string base, const std::string findStr, const std::size_t pos) {

		std::string newBase = base, newfindStr = findStr;
		std::transform(begin(base), end(base), begin(newBase), [](unsigned char c) { return std::tolower(c); });
		std::transform(begin(findStr), end(findStr), begin(newfindStr), [](unsigned char c) { return std::tolower(c); });

		return newBase.find(newfindStr, pos);
	}
};
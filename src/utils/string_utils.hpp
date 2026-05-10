#pragma once

#include <string>
#include <algorithm>
#include <cctype>
#include <optional>
#include <functional>

namespace string_utils {
	std::optional<std::size_t> isThereAny(const std::string& base, const std::size_t& baseIndex, const std::string list[], const std::size_t listSize);
	std::optional<std::string> findNextWord(const std::string& base, const std::size_t& index, const bool startInWord = false, const std::function<bool(char)> isLetter = [](char ch) {return !std::isspace(ch);});
};
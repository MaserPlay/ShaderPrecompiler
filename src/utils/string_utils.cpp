#include "string_utils.hpp"

#include <string>

namespace string_utils {
	std::optional<std::size_t> isThereAny(const std::string& base, const std::size_t& index, const std::string list[], const std::size_t listSize) {
		for (std::size_t d = 0; d < listSize; d++)
		{
			std::string directive = list[d];
			bool isWord = false;
			if (base.size() >= index + directive.size()) {
				for (std::size_t ii = 0; ii < directive.size(); ii++)
				{
					if (directive[ii] == base[ii + index]) {
						isWord = true;
					}
					else {
						isWord = false;
						break;
					}
				}
			}

			if (isWord) {
				return d;
			}
		}

		return std::nullopt;
	}
	std::optional<std::string> findNextWord(const std::string& base, const std::size_t& index, const bool startInWord, const std::function<bool(char)> isLetter) {
		std::string word{};
		bool inWord = startInWord;
		for (std::size_t wi = index; wi < base.size(); wi++)
		{
			if (isLetter(base[wi])) {
				if (!inWord) {
					inWord = true;
				}
				word += base[wi];
			}
			else {
				if (inWord) {
					return word;
				}
			}
		}
		return std::nullopt;
	}
};
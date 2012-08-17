#include "String.h"

using namespace utilities;

bool string::startsWith(const std::string& text, const std::string& startsWith, bool caseInsensitive) {
	if (text.length() < startsWith.length() || text.empty() || startsWith.empty()) {
		return false;
	}

	// Case sensitive
	if (!caseInsensitive) {
		for (size_t i = 0; i < startsWith.size(); ++i) {
			if (text[i] != startsWith[i]) {
				return false;
			}
		}
	}
	// Case-insensitive
	else {
		std::string textLower = text;
		string::toLower(textLower);
		std::string startsWithLower = startsWith;
		string::toLower(startsWithLower);

		for (size_t i = 0; i < startsWith.size(); ++i) {
			if (text[i] != startsWith[i]) {
				return false;
			}
		}
	}

	return true;
}
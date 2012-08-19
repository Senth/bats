#pragma once

#include <string>
#include <sstream>
#include <algorithm>

namespace utilities {
namespace string {
	/**
	 * Trims the beginning of a string, by default it removes all whitespace characters
	 * @param[in] source the variable to trim the beginning of
	 * @param[in] remove all characters that are to be removed, defaults to all whitespace
	 * characters.
	 * @return the string trimmed at the beginning.
	 */
	inline std::string trimLeft(const std::string& source, const std::string& remove = " \t\r\n") {
		if (source.empty()) {
			return source;
		}
		size_t trimPos = source.find_first_not_of(remove);
		if (trimPos == source.npos) {
			return std::string();
		} else {
			return source.substr(trimPos);
		}
	}

	/**
	 * Trims the end of a string, by default it removes all whitespace characters
	 * @param[in] source the variable to trim the beginning of
	 * @param[in] remove all characters that are to be removed, defaults to all whitespace
	 * characters.
	 * @return the string trimmed at the end.
	 */
	inline std::string trimRight(const std::string& source, const std::string& remove = " \t\r\n") {
		if (source.empty()) {
			return source;
		}
		size_t trimPos = source.find_last_not_of(remove);
		if (trimPos == source.npos) {
			return std::string();
		} else {
			return source.substr(0, trimPos + 1);
		}
	}

	/**
	 * Trims both the beginning and end of a string, by default it removes all whitespace
	 * characters.
	 * @param[in] source the variable to trim the beginning and end of
	 * @param[in] remove all characters that are to be removed, defaults to all whitespace
	 * characters.
	 * @return the trimmed string
	 */
	inline std::string trim(const std::string& source, const std::string& remove = " \t\r\n") {
		return trimRight(trimLeft(source, remove), remove);
	}

	/**
	 * Converts the text string to lowercase characters.
	 * @param text the text to convert to lowercase.
	 */
	inline void toLower(std::string& text) {
		std::transform(text.begin(), text.end(), text.begin(), ::tolower);
	}

	/**
	 * Converts the text string to uppercase characters.
	 * @param text the text to convert to lowercase.
	 */
	inline void toUpper(std::string& text) {
		std::transform(text.begin(), text.end(), text.begin(), ::toupper);
	}

	/**
	 * Converts the type to a string
	 * @param value the value to get as a string
	 * @tparam the type to convert from
	 */
	template<typename T>
	std::string toString(const T& value) {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	/**
	 * Checks if a string starts with the other string
	 * @param text the whole text to check if it starts with "startsWith"
	 * @param startWith the text should start with this
	 * @param caseInsensitive (optional) set to true if you want do an
	 * case insensitive comparison (not effective)
	 * @return true if text starts with the string "startWith"
	 */
	bool startsWith(const std::string& text, const std::string& startsWith, bool caseInsensitive = false);
	
}
}
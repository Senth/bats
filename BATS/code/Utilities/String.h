#pragma once

#include <string>

namespace utilities {
namespace string {
	/**
	 * Trims the beginning of a string, by default it removes all whitespace characters
	 * @param[in] source the variable to trim the beginning of
	 * @param[in] remove all characters that are to be removed, defaults to all whitespace
	 * characters.
	 * @return the string trimmed at the beginning.
	 */
	inline std::string trimLeft(const std::string& source, const std::string& remove = " \r\n") {
		std::string trimmed = source;
		return trimmed.erase(0, trimmed.find_first_not_of(remove));
	}

	/**
	 * Trims the end of a string, by default it removes all whitespace characters
	 * @param[in] source the variable to trim the beginning of
	 * @param[in] remove all characters that are to be removed, defaults to all whitespace
	 * characters.
	 * @return the string trimmed at the end.
	 */
	inline std::string trimRight(const std::string& source, const std::string& remove = " \r\n") {
		std::string trimmed = source;
		return trimmed.erase(trimmed.find_last_not_of(remove) + 1);
	}

	/**
	 * Trims both the beginning and end of a string, by default it removes all whitespace
	 * characters.
	 * @param[in] source the variable to trim the beginning and end of
	 * @param[in] remove all characters that are to be removed, defaults to all whitespace
	 * characters.
	 * @return the trimmed string
	 */
	inline std::string trim(const std::string& source, const std::string& remove = " \r\n") {
		return trimRight(trimLeft(source, remove), remove);
	}
}
}
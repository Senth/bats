#include "IniReader.h"
#include <sstream>
#include <algorithm>
#include "Logger.h"
#include "String.h"

using namespace utilities;
using std::stringstream;

VariableInfo::operator bool() const {
	std::string lowerValue = value;
	std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);

	if (lowerValue == "true" || lowerValue == "1") {
		return true;
	} else if (lowerValue == "false" || lowerValue == "0") {
		return false;
	} else {
		ERROR_MESSAGE(false, "VariableInfo::bool() | Could not transform string value '" <<
			value << "' to bool!");
		return false;
	}
}

VariableInfo::operator int() const {
	stringstream ss(value);
	int intValue = 0;
	ss >> intValue;

	if (!ss.good() && !ss.eof()) {
		ERROR_MESSAGE(false, "VariableInfo::int() | Could not transform string value '" <<
			value << "' to int!");
		return 0;
	}

	return intValue;
}

VariableInfo::operator float() const {
	stringstream ss(value);
	float floatValue = 0.0f;
	ss >> floatValue;

	if (!ss.good() && !ss.eof()) {
		ERROR_MESSAGE(false, "VariableInfo::int() | Could not transform string value '" <<
			value << "' to float!");
		return 0.0f;
	}

	return floatValue;
}

VariableInfo::operator double() const {
	stringstream ss(value);
	double doubleValue = 0.0;
	ss >> doubleValue;

	if (!ss.good() && !ss.eof()) {
		ERROR_MESSAGE(false, "VariableInfo::int() | Could not transform string value '" <<
			value << "' to double!");
		return 0.0;
	}

	return doubleValue;
}

IniReader::IniReader() {
	// Does nothing
}

IniReader::IniReader(const std::string& filePath) {
	open(filePath);
}

IniReader::~IniReader() {
	// Does nothing
}

void IniReader::open(const std::string& filePath) {
	mGood = true;
	mFile.open(filePath.c_str());

	mSectionCurrent = "";
	mSubSectionCurrent = "";
	mFilename = convertToFilebase(filePath);
	mNextVariable.file = mFilename;

	readNext();
}

void IniReader::close() {
	mFile.close();
	mFile.clear();
}

bool IniReader::isOpen() const {
	return mFile.is_open();
}

bool IniReader::isGood() const {
	if (mFile.is_open() && mGood) {
		return true;
	} else {
		return false;
	}
}

bool IniReader::readNext(VariableInfo& variableInfo) {
	if (mGood) {
		variableInfo = mNextVariable;
		readNext();
		return true;
	} else {
		return false;
	}
}

std::string IniReader::convertToFilebase(const std::string& filename) const {
	std::string filebase = filename;

	// Remove directory
	size_t dirEndPos = filebase.find_last_of("/\\");
	if (dirEndPos != filebase.npos) {
		filebase = filebase.substr(dirEndPos+1);
	}

	// Remove .ini extension (or possibly another extension)
	size_t extEndPos = filebase.find_last_of('.');
	if (extEndPos != filebase.npos) {
		filebase = filebase.substr(0, extEndPos);
	}

	return filebase;
}

void IniReader::readNext() {
	bool readVariable = false;

	// Continue reading until the next variable is found
	while (!readVariable) {
		std::string line;
		getline(mFile, line);

		// Failed to read line
		if (!mFile.good()) {
			mGood = false;
			break;
		}

		removeComment(line);

		// Skip empty lines
		if (line.empty()) {
			continue;
		}

		// Section or subsection when line starts with [
		if (line[0] == '[') {
			// remove [ and ].
			line = line.substr(1);

			size_t rightBracketPos = line.find_last_of(']');
			if (rightBracketPos != line.npos) {
				line = line.substr(0, rightBracketPos);

				// Is it a subsection?
				size_t dotPos = line.find_first_of('.');

				// Section
				if (dotPos == line.npos) {
					mSectionCurrent = line;
					mSubSectionCurrent.clear();
				}
				// Subsection
				else {
					// Split section and subsection and set both.
					mSectionCurrent = line.substr(0, dotPos);
					mSubSectionCurrent = line.substr(dotPos+1);
				}
			}
			// Syntax error
			else {
				ERROR_MESSAGE(false, "IniReader::readNext() | Syntax error: could not find " <<
					"ending ] for section: " << line);
			}
		} else {
			// Check whether this is a variable name or an empty line with whitespace
			size_t equalPos = line.find_first_of('=');

			if (equalPos != line.npos) {
				// Split variable name and value
				mNextVariable.name = line.substr(0, equalPos);
				mNextVariable.value = line.substr(equalPos+1);

				// Trim whitespace
				mNextVariable.name = utilities::string::trim(mNextVariable.name);
				mNextVariable.value = utilities::string::trim(mNextVariable.value);

				// Check so that there actually exists a variable name and value,
				// Just skip the variable and try to read the rest, don't abort.
				if (mNextVariable.name.empty()) {
					std::string sectionName;
					if (mSubSectionCurrent.empty()) {
						sectionName = mSectionCurrent;
					} else {
						sectionName = mSectionCurrent;
						sectionName += ".";
						sectionName += mSubSectionCurrent;
					}
					ERROR_MESSAGE(false, "IniReader::readNext() | Syntax error, missing " <<
						"variable name in " << mFilename << ".ini [" << sectionName << "]!");
				} else if (mNextVariable.value.empty()) {
					std::string sectionName;
					if (mSubSectionCurrent.empty()) {
						sectionName = mSectionCurrent;
					} else {
						sectionName = mSectionCurrent;
						sectionName += ".";
						sectionName += mSubSectionCurrent;
					}
					ERROR_MESSAGE(false, "IniReader::readNext() | Syntax error, missing " <<
						"variable value for '" << mNextVariable.name << "' in " << mFilename <<
						".ini [" << sectionName << "]!");
				}
				// Good variable name and value
				else {
					readVariable = true;
				}
			}
		}
	}

	// Set section values if we read an variable
	if (readVariable) {
		mNextVariable.section = mSectionCurrent;
		mNextVariable.subsection = mSubSectionCurrent;
	} else {
		mGood = false;
	}
}

void IniReader::removeComment(std::string& line) const {
	size_t commentSignPos = line.find_first_of("#;");

	if (commentSignPos != line.npos) {
		line = line.substr(0, commentSignPos);
	}
}
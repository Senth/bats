#include "MassIniReader.h"

using namespace utilities;

const std::string INI_EXTENTION = "ini";

MassIniReader::MassIniReader() {
	mOpened = false;
}

MassIniReader::MassIniReader(
	const std::string& directory,
	bool recursive,
	const std::string& regex)
{
	mOpened = false;
	open(directory, recursive, regex);
}

MassIniReader::~MassIniReader() {
	// Does nothing
}

void MassIniReader::open(
	const std::string& directory,
	bool recursive,
	const std::string& regex)
{
	mDirectory = directory;
	mRecursive = recursive;
	mRegex = regex;

	findAllFiles(mDirectory);

	// Open the first file
	if (!mFileQueue.empty()) {
		IniReader::open(mFileQueue.front());
		mFileQueue.pop_front();
	}
}

void MassIniReader::close() {
	IniReader::close();
	mFileQueue.clear();
	mDirectory.clear();
	mRegex.clear();
	mOpened = false;
}

bool MassIniReader::isOpen() const {
	return mOpened;
}

bool MassIniReader::isGood() const {
	return IniReader::isGood();
}

bool MassIniReader::readNext(VariableInfo& variableInfo) {
	IniReader::readNext(variableInfo);

	// While we don't have a next variable try to find one in another file
	while (!isGood() && !mFileQueue.empty()) {
		IniReader::close();
		IniReader::open(mFileQueue.front());
		mFileQueue.pop_front();
	}

	// We should have a good variable now, if not we have no variables left for all files
	return isGood();
}

void MassIniReader::nextFile() {
	IniReader::close();

	if (!mFileQueue.empty()) {
		IniReader::open(mFileQueue.front());
		mFileQueue.pop_front();
	}
}

void MassIniReader::findAllFiles(const std::string& directory) {
#ifdef WINDOWS
	HANDLE hFile = NULL;
	WIN32_FIND_DATA fileInformation;

	std::string searchPattern = directory + "\\*.*";

	// Find directory
	hFile = FindFirstFile(searchPattern.c_str(), &fileInformation);
	if(hFile != INVALID_HANDLE_VALUE) {
		mOpened = true;

		// Find all files inside the directory
		do {
			// Skip hidden files
			if(fileInformation.cFileName[0] != '.') {
				std::string filePath = directory + "\\" + fileInformation.cFileName;

				// Directory
				if(fileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if(mRecursive) {
						// Search subdirectory
						findAllFiles(filePath);
					}
				}
				// Regular file
				else {
					// Check extension
					std::string extension = fileInformation.cFileName;
					extension = extension.substr(extension.rfind(".") + 1);

					if(extension == INI_EXTENTION)
					{
						///@todo regular expressions check

						// Save filename
						mFileQueue.push_back(filePath);
					}
				}
			}
		} while(FindNextFile(hFile, &fileInformation) == TRUE);

		// Close handle
		FindClose(hFile);
	}
#endif
}